/**

  MemoryMap helper functions.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/OcDebugLogLib.h>
#include <Library/OcMiscLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>

#include "Config.h"
#include "MemoryMap.h"
#include "CustomSlide.h"
#include "ServiceOverrides.h"

STATIC CHAR16 *mEfiMemoryTypeDesc[EfiMaxMemoryType] = {
  L"Reserved",
  L"LDR_code",
  L"LDR_data",
  L"BS_code",
  L"BS_data",
  L"RT_code",
  L"RT_data",
  L"Available",
  L"Unusable",
  L"ACPI_recl",
  L"ACPI_NVS",
  L"MemMapIO",
  L"MemPortIO",
  L"PAL_code"
};

/** AMI CSM module allocates up to two regions for legacy video output.
 *  1. For PMM and EBDA areas.
 *     On Ivy Bridge and below it ends at 0xA0000-0x1000-0x1 and has EfiBootServicesCode type.
 *     On Haswell and above it is allocated below 0xA0000 address with the same type.
 *  2. For Intel RC S3 reserved area, fixed from 0x9F000 to 0x9FFFF.
 *     On Sandy Bridge and below it is not present in memory map.
 *     On Ivy Bridge and newer it is present as EfiRuntimeServicesData.
 *     Starting from at least SkyLake it is present as EfiReservedMemoryType.
 *
 *  Prior to AptioMemoryFix EfiRuntimeServicesData could have been relocated by boot.efi,
 *  and the 2nd region could have been overwritten by the kernel. Now it is no longer the
 *  case, and only the 1st region may need special handling.
 *
 *  For the 1st region there appear to be (unconfirmed) reports that it may still be accessed
 *  after waking from sleep. This does not seem to be valid according to AMI code, but we still
 *  protect it in case such systems really exist.
 *
 *  Researched and fixed on gigabyte boards by Slice
 */
VOID
ProtectCsmRegion (
  UINTN                  MemoryMapSize,
  EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  UINTN                  DescriptorSize
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  UINTN                   BlockSize;
  UINTN                   PhysicalEnd;

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;

  for (Index = 0; Index < NumEntries; Index++) {
    BlockSize = EFI_PAGES_TO_SIZE ((UINTN)Desc->NumberOfPages);
    PhysicalEnd = Desc->PhysicalStart + BlockSize;

    if (PhysicalEnd >= 0x9E000 && PhysicalEnd < 0xA0000 && Desc->Type == EfiBootServicesData) {
      Desc->Type = EfiACPIMemoryNVS;
      break;
    }

    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
  }
}

VOID
PrintMemMap (
  IN CONST CHAR16           *Name,
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN EFI_PHYSICAL_ADDRESS   SysTable
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  UINT64                  Bytes;
  EFI_MEMORY_DESCRIPTOR   *Desc;

  //
  // Printing onscreen may allocate the memory internally.
  // This is very dangerous to do in GetMemoryMap or SetVirtualAddresses wrappers,
  // because on many ASUS boards the internal memory map will get modified, and
  // for some reason this will cause crashes right after os boots.
  //
  DisableDynamicPoolAllocations ();

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;
  OcPrintScreen (L"--- Dump Memory Map (%s) start ---\n", Name);
  OcPrintScreen (L"MEMMAP: Size=%d, Addr=%p, DescSize=%d, ST=%08lX\n",
    MemoryMapSize, MemoryMap, DescriptorSize, (UINTN)SysTable);
  OcPrintScreen (L"Type      Start      End        Virtual          # Pages    Attributes\n");
  for (Index = 0; Index < NumEntries; Index++) {

    Bytes = EFI_PAGES_TO_SIZE (Desc->NumberOfPages);

    OcPrintScreen (L"%-9s %010lX %010lX %016lX %010lX %016lX\n",
      mEfiMemoryTypeDesc[Desc->Type],
      Desc->PhysicalStart,
      Desc->PhysicalStart + Bytes - 1,
      Desc->VirtualStart,
      Desc->NumberOfPages,
      Desc->Attribute
    );
    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
    //
    // It is often the case that memory map does not fit onscreen.
    // There is no way to reliably determine the largest console window, so we just stall
    // for a moment to let one read the output.
    //
    if ((Index + 1) % 16 == 0)
      gBS->Stall (SECONDS_TO_MICROSECONDS (5));
  }

  OcPrintScreen (L"--- Dump Memory Map (%s) end ---\n", Name);
  gBS->Stall (SECONDS_TO_MICROSECONDS (5));

  EnableDynamicPoolAllocations ();
}
