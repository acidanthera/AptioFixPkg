/**

  Runtime Services Wrappers.

  by Download-Fritz & vit9696

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/OcMiscLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Guid/GlobalVariable.h>
#include <Guid/OcVariables.h>

#include <Protocol/OcFirmwareRuntime.h>

#include "Config.h"
#include "RtShims.h"
#include "BootFixes.h"
#include "MemoryMap.h"

STATIC RT_RELOC_PROTECT_DATA mRelocInfoData;
STATIC EFI_GET_VARIABLE      mCustomGetVariable;
STATIC EFI_GET_VARIABLE      mOrgGetVariable;
STATIC EFI_EVENT             mCustomGetVariableEvent;

EFI_STATUS
EFIAPI
OrgGetVariable (
  IN     CHAR16    *VariableName,
  IN     EFI_GUID  *VendorGuid,
  OUT    UINT32    *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT    VOID      *Data
  )
{
  return (mOrgGetVariable != NULL ? mOrgGetVariable : gRT->GetVariable) (
    VariableName,
    VendorGuid,
    Attributes,
    DataSize,
    Data
    );
}

/** Protect RT data from relocation by marking them MemMapIO. Except area with EFI system table.
 *  This one must be relocated into kernel boot image or kernel will crash (kernel accesses it
 *  before RT areas are mapped into vm).
 *  This fixes NVRAM issues on some boards where access to nvram after boot services is possible
 *  only in SMM mode. RT driver passes data to SM handler through previously negotiated buffer
 *  and this buffer must not be relocated.
 *  Explained and examined in detail by CodeRush and night199uk:
 *  http://www.projectosx.com/forum/index.php?showtopic=3298
 *
 *  It seems this does not do any harm to others where this is not needed,
 *  so it's added as standard fix for all.
 *
 *  Starting with APTIO V for nvram to work not only data but could too can no longer be moved
 *  due to the use of commbuffers. This, however, creates a memory protection issue, because
 *  XNU maps RT data as RW and code as RX, and AMI appears use global variables in some RT drivers.
 *  For this reason we shim (most?) affected RT services via wrapers that unset the WP bit during
 *  the UEFI call and set it back on return.
 *  Explained in detail by Download-Fritz and vit9696:
 *  http://www.insanelymac.com/forum/topic/331381-aptiomemoryfix (first 2 links in particular).
 */
VOID
ProtectRtMemoryFromRelocation (
  IN     UINTN                  MemoryMapSize,
  IN     UINTN                  DescriptorSize,
  IN     UINT32                 DescriptorVersion,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN     EFI_PHYSICAL_ADDRESS   SysTableArea
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  RT_RELOC_PROTECT_INFO   *RelocInfo;

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;

  mRelocInfoData.NumEntries = 0;

  RelocInfo = &mRelocInfoData.RelocInfo[0];

  for (Index = 0; Index < NumEntries; Index++) {
    if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0 &&
        (Desc->Type == EfiRuntimeServicesCode ||
        (Desc->Type == EfiRuntimeServicesData && Desc->PhysicalStart != SysTableArea))) {

      if (mRelocInfoData.NumEntries < ARRAY_SIZE (mRelocInfoData.RelocInfo)) {
        RelocInfo->PhysicalStart = Desc->PhysicalStart;
        RelocInfo->Type          = Desc->Type;
        ++RelocInfo;
        ++mRelocInfoData.NumEntries;
      } else {
        DEBUG ((DEBUG_INFO, "WARNING: Cannot save mem type for entry: %lx (type 0x%x)\n", Desc->PhysicalStart, (UINTN)Desc->Type));
      }

      DEBUG ((DEBUG_VERBOSE, "RT mem %lx (0x%x) -> MemMapIO\n", Desc->PhysicalStart, Desc->NumberOfPages));
      Desc->Type = EfiMemoryMappedIO;
    }

    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
  }
}

VOID
RestoreProtectedRtMemoryTypes (
  IN     UINTN                  MemoryMapSize,
  IN     UINTN                  DescriptorSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap
  )
{
  UINTN Index;
  UINTN Index2;
  UINTN NumEntriesLeft;

  NumEntriesLeft = mRelocInfoData.NumEntries;

  if (NumEntriesLeft > 0) {
    for (Index = 0; Index < (MemoryMapSize / DescriptorSize); ++Index) {
      if (NumEntriesLeft > 0) {
        for (Index2 = 0; Index2 < mRelocInfoData.NumEntries; ++Index2) {
          if (MemoryMap->PhysicalStart == mRelocInfoData.RelocInfo[Index2].PhysicalStart) {
            MemoryMap->Type = mRelocInfoData.RelocInfo[Index2].Type;
            --NumEntriesLeft;
          }
        }
      }

      MemoryMap = NEXT_MEMORY_DESCRIPTOR (MemoryMap, DescriptorSize);
    }
  }
}

VOID
EFIAPI
SetGetVariableHookHandler (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                    Status;
  OC_FIRMWARE_RUNTIME_PROTOCOL  *FwRuntime;

  if (mOrgGetVariable == NULL) {
    Status = gBS->LocateProtocol (
      &gOcFirmwareRuntimeProtocolGuid,
      NULL,
      (VOID **) &FwRuntime
      );

    if (!EFI_ERROR (Status) && FwRuntime->Revision == OC_FIRMWARE_RUNTIME_REVISION) {
      FwRuntime->OnGetVariable (mCustomGetVariable, &mOrgGetVariable);
    }
  }
}

VOID
InstallRtShims (
  EFI_GET_VARIABLE GetVariableOverride
  )
{
  EFI_STATUS  Status;
  VOID        *Registration;

  mCustomGetVariable = GetVariableOverride;

  SetGetVariableHookHandler (NULL, NULL);

  if (mOrgGetVariable != NULL) {
    return;
  }

  Status = gBS->CreateEvent (
    EVT_NOTIFY_SIGNAL,
    TPL_NOTIFY,
    SetGetVariableHookHandler,
    NULL,
    &mCustomGetVariableEvent
    );

  if (!EFI_ERROR (Status)) {
    Status = gBS->RegisterProtocolNotify (
      &gOcFirmwareRuntimeProtocolGuid,
      mCustomGetVariableEvent,
      &Registration
      );

    if (EFI_ERROR (Status)) {
      gBS->CloseEvent (&mCustomGetVariableEvent);
    }
  }
}
