/**

  Temporary BS and RT overrides for boot.efi support.
  Unlike RtShims they do not affect the kernel.

  by dmazar

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "Config.h"
#include "BootArgs.h"
#include "BootFixes.h"
#include "Hibernate.h"
#include "Lib.h"
#include "RtShims.h"
#include "ServiceOverrides.h"

// Placeholders for storing original Boot and RT Services functions
EFI_ALLOCATE_PAGES          gStoredAllocatePages        = NULL;
EFI_ALLOCATE_POOL           gStoredAllocatePool         = NULL;
EFI_FREE_POOL               gStoredFreePool             = NULL;
EFI_GET_MEMORY_MAP          gStoredGetMemoryMap         = NULL;
EFI_EXIT_BOOT_SERVICES      gStoredExitBootServices     = NULL;
EFI_HANDLE_PROTOCOL         gStoredHandleProtocol       = NULL;
EFI_SET_VIRTUAL_ADDRESS_MAP gStoredSetVirtualAddressMap = NULL;
UINT32                      mRtPreOverridesCRC32 = 0;

EFI_PHYSICAL_ADDRESS        gMinAllocatedAddr = 0;
EFI_PHYSICAL_ADDRESS        gMaxAllocatedAddr = 0;

// Location of memory allocated by boot.efi for hibernate image
EFI_PHYSICAL_ADDRESS        gHibernateImageAddress = 0;

// Last descriptor size obtained from GetMemoryMap
UINTN                       gMemoryMapDescriptorSize = sizeof(EFI_MEMORY_DESCRIPTOR);

// saved exit boot services arguments
EFI_HANDLE                  gExitBSImageHandle = 0;
UINTN                       gExitBSMapKey       = 0; 

VOID
InstallBsOverrides (
  VOID
  )
{
  gStoredAllocatePages    = gBS->AllocatePages;
  //gStoredAllocatePool     = gBS->AllocatePool;
  //gStoredFreePool         = gBS->FreePool;
  gStoredGetMemoryMap     = gBS->GetMemoryMap;
  gStoredExitBootServices = gBS->ExitBootServices;
  gStoredHandleProtocol   = gBS->HandleProtocol;

  gBS->AllocatePages    = MOAllocatePages;
  //gBS->AllocatePool     = MOAllocatePool;
  //gBS->FreePool         = MOFreePool;
  gBS->GetMemoryMap     = MOGetMemoryMap;
  gBS->ExitBootServices = MOExitBootServices;
  gBS->HandleProtocol   = MOHandleProtocol;

  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
}

VOID
UninstallBsOverrides (
  VOID
  )
{
  gBS->AllocatePages    = gStoredAllocatePages;
  //gBS->AllocatePool     = gStoredAllocatePool;
  //gBS->FreePool         = gStoredFreePool;
  gBS->GetMemoryMap     = gStoredGetMemoryMap;
  gBS->ExitBootServices = gStoredExitBootServices;
  gBS->HandleProtocol   = gStoredHandleProtocol;

  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);
}

VOID
InstallRtOverrides (
  VOID
  )
{
  mRtPreOverridesCRC32 = gRT->Hdr.CRC32;

  gStoredSetVirtualAddressMap = gRT->SetVirtualAddressMap;

  gRT->SetVirtualAddressMap = MOSetVirtualAddressMap;

  gRT->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
}

VOID
UninstallRtOverrides (
  VOID
  )
{
  gRT->SetVirtualAddressMap = gStoredSetVirtualAddressMap;

  gRT->Hdr.CRC32 = mRtPreOverridesCRC32;
}

/** gBS->HandleProtocol override:
 * Boot.efi requires EfiGraphicsOutputProtocol on ConOutHandle, but it is not present
 * there on Aptio 2.0. EfiGraphicsOutputProtocol exists on some other handle.
 * If this is the case, we'll intercept that call and return EfiGraphicsOutputProtocol
 * from that other handle.
 */
EFI_STATUS
EFIAPI
MOHandleProtocol (
  IN EFI_HANDLE    Handle,
  IN EFI_GUID      *Protocol,
  OUT VOID         **Interface
  )
{
  EFI_STATUS                    Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

  Status = gStoredHandleProtocol (Handle, Protocol, Interface);

  if (EFI_ERROR (Status) && CompareGuid (Protocol, &gEfiGraphicsOutputProtocolGuid)) {
    //
    // Let's find it on some other handle
    //
    Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&GraphicsOutput);
    if (Status == EFI_SUCCESS) {
      *Interface = GraphicsOutput;
      DEBUG ((DEBUG_VERBOSE, "HandleProtocol(%p, %s, %p) = %r (from other handle)\n", Handle, GuidStr(Protocol), *Interface, Status));
    }
  }

  return Status;
}

/** gBS->AllocatePages override:
 * Returns pages from free memory block to boot.efi for kernel boot image.
 */
EFI_STATUS
EFIAPI
MOAllocatePages (
  IN EFI_ALLOCATE_TYPE         Type,
  IN EFI_MEMORY_TYPE           MemoryType,
  IN UINTN                     NumberOfPages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory
  )
{
  EFI_STATUS              Status;
  EFI_PHYSICAL_ADDRESS    UpperAddr;

  if (Type == AllocateAddress && MemoryType == EfiLoaderData) {
    //
    // Called from boot.efi
    //
    UpperAddr = *Memory + EFI_PAGES_TO_SIZE (NumberOfPages);

    //
    // Store min and max mem - can be used later to determine start and end of kernel boot image
    //
    if (gMinAllocatedAddr == 0 || *Memory < gMinAllocatedAddr)
      gMinAllocatedAddr = *Memory;
    if (UpperAddr > gMaxAllocatedAddr)
      gMaxAllocatedAddr = UpperAddr;

    Status = gStoredAllocatePages (Type, MemoryType, NumberOfPages, Memory);
  } else if (gHibernateWake && Type == AllocateAnyPages && MemoryType == EfiLoaderData) {
    //
    // Called from boot.efi during hibernate wake,
    // first such allocation is for hibernate image
    //
    Status = gStoredAllocatePages (Type, MemoryType, NumberOfPages, Memory);
    if (gHibernateImageAddress == 0 && Status == EFI_SUCCESS) {
      gHibernateImageAddress = *Memory;
    }
  } else {
    //
    // Generic page allocation
    //
    Status = gStoredAllocatePages (Type, MemoryType, NumberOfPages, Memory);
  }

  return Status;
}

/** gBS->AllocatePool override:
 * Returns pages from top addresses for boot.efi in order to avoid collisions with kernel boot image.
 */
EFI_STATUS
EFIAPI
MOAllocatePool (
  IN EFI_MEMORY_TYPE       Type,
  IN  UINTN                Size,
  OUT VOID                 **Buffer
  )
{
  //TODO: You may try implementing AllocatePagesFromTop wrapper here
  // for APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION support to workaround boot.efi
  // allocations in the kernel image area. MOFreePool and MOAllocatePages with AllocateAnyPages
  // will also need to be changed. Some static array tracking the pointers & sizes may do?
  return gStoredAllocatePool (Type, Size, Buffer);
}

/** gBS->FreePool override:
 * Returns pages from top addresses for boot.efi in order to avoid collisions with kernel boot image.
 */
EFI_STATUS
EFIAPI
MOFreePool (
  IN VOID                 *Buffer
  )
{
  return gStoredFreePool (Buffer);
}

/** gBS->GetMemoryMap override:
 * Returns shrinked memory map. XNU can handle up to PMAP_MEMORY_REGIONS_SIZE (128) entries.
 */
EFI_STATUS
EFIAPI
MOGetMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  OUT UINTN                     *MapKey,
  OUT UINTN                     *DescriptorSize,
  OUT UINT32                    *DescriptorVersion
  )
{
  EFI_STATUS            Status;

  Status = gStoredGetMemoryMap (MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
  DEBUG ((DEBUG_VERBOSE, "GetMemoryMap: %p = %r\n", MemoryMap, Status));

  if (Status == EFI_SUCCESS) {
    if (gDumpMemArgPresent)
      PrintMemMap (L"GetMemoryMap", *MemoryMapSize, *DescriptorSize, MemoryMap, gRtShims, gSysTableRtArea);

#if APTIOFIX_PROTECT_CSM_REGION == 1
    ProtectCsmRegion (*MemoryMapSize, MemoryMap, *DescriptorSize);
#endif

    ShrinkMemMap (MemoryMapSize, MemoryMap, *DescriptorSize);

    //
    // Remember some descriptor size, since we will not have it later
    // during hibernate wake to be able to iterate memory map.
    //
    gMemoryMapDescriptorSize = *DescriptorSize;
  }

  return Status;
}

/** gBS->ExitBootServices override:
 * Patches kernel entry point with jump to our KernelEntryPatchJumpBack().
 */
EFI_STATUS
EFIAPI
MOExitBootServices (
  IN EFI_HANDLE     ImageHandle,
  IN UINTN          MapKey
  )
{
  EFI_STATUS               Status;
  UINTN                    SlideAddr = 0;
  VOID                     *MachOImage = NULL;
  IOHibernateImageHeader   *ImageHeader = NULL;

  //
  // We need hibernate image address for wake
  //
  if (gHibernateWake && gHibernateImageAddress == 0) {
    PrintScreen (L"AMF: Failed to find hibernate image address\n");
    gBS->Stall (5000000);
    return EFI_INVALID_PARAMETER;
  }

  //
  // We can just return EFI_SUCCESS and continue using Print for debug
  //
  if (gDumpMemArgPresent) {
    gExitBSImageHandle = ImageHandle;
    gExitBSMapKey      = MapKey; 
    Status             = EFI_SUCCESS;
  } else {
    Status = ForceExitBootServices (gStoredExitBootServices, ImageHandle, MapKey);
  }

  DEBUG ((DEBUG_VERBOSE, "ExitBootServices %r\n", Status));

  if (EFI_ERROR(Status))
    return Status;

  if (!gHibernateWake) {
    DEBUG ((DEBUG_VERBOSE, "ExitBootServices: gMinAllocatedAddr: %lx, gMaxAllocatedAddr: %lx\n", gMinAllocatedAddr, gMaxAllocatedAddr));

    SlideAddr  = gMinAllocatedAddr - 0x100000;
    MachOImage = (VOID*)(UINTN)(SlideAddr + 0x200000);
    KernelEntryFromMachOPatchJump (MachOImage, SlideAddr);
  } else {
    //
    // At this stage HIB section is not yet copied from sleep image to it's
    // proper memory destination. so we'll patch entry point in sleep image.
    //
    ImageHeader = (IOHibernateImageHeader *)(UINTN)gHibernateImageAddress;
    KernelEntryPatchJump (
      ((UINT32)(UINTN)&(ImageHeader->fileExtentMap[0])) + ImageHeader->fileExtentMapSize + ImageHeader->restore1CodeOffset
      );
  }

  return Status;
}

/** gRT->SetVirtualAddressMap override:
 * Fixes virtualizing of RT services.
 */
EFI_STATUS
EFIAPI
MOSetVirtualAddressMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *VirtualMap
  )
{
  EFI_STATUS   Status;
  UINT32       EfiSystemTable;

  DEBUG ((DEBUG_VERBOSE, "SetVirtualAddressMap(%d, %d, 0x%x, %p) START ...\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap));

  //
  // We do not need to recover BS, since they will be invalid.
  //
  UninstallRtOverrides ();

  if (gDumpMemArgPresent) {
    PrintMemMap (L"SetVirtualAddressMap", MemoryMapSize, DescriptorSize, VirtualMap, gRtShims, gSysTableRtArea);
    //
    // To print as much information as possible we delay ExitBootServices.
    // Most likely this will fail, but let's still try!
    //
    ForceExitBootServices (gStoredExitBootServices, gExitBSImageHandle, gExitBSMapKey);
  }

  //
  // Protect RT areas from relocation by marking then MemMapIO
  //
  ProtectRtMemoryFromRelocation (MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);

  //
  // Remember physical sys table addr
  //
  EfiSystemTable = (UINT32)(UINTN)gST;

  //
  // Virtualize RT services with all needed fixes
  //
  Status = ExecSetVirtualAddressesToMemMap (MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);

  CopyEfiSysTableToSeparateRtDataArea (&EfiSystemTable);

  //
  // Correct shim pointers right away
  //
  VirtualizeRtShims (MemoryMapSize, DescriptorSize, VirtualMap);

  return Status;
}
