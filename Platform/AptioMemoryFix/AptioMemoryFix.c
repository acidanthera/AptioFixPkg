/**

 UEFI driver for enabling loading of macOS without memory relocation.

 by dmazar

 **/

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/CpuLib.h>

#include <Guid/GlobalVariable.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/AptioMemoryFixProtocol.h>

#include "Config.h"
#include "BootArgs.h"
#include "BootFixes.h"
#include "AsmFuncs.h"
#include "VMem.h"
#include "Lib.h"
#include "Hibernate.h"
#include "RtShims.h"

// One could discover AptioMemoryFix with this protocol.
STATIC APTIOMEMORYFIX_PROTOCOL mAptioMemoryFixProtocol = {
  APTIOMEMORYFIX_PROTOCOL_REVISION
};

STATIC UINTN                mCounter = 0;

// TRUE if we are doing hibernate wake
BOOLEAN                     gHibernateWake = FALSE;

// TRUE if booting with a manually specified slide=X
BOOLEAN                     gSlideArgPresent = FALSE;

// TRUE if booting with -aptiodump
BOOLEAN                     gDumpMemArgPresent = FALSE;

// placeholders for storing original Boot and RT Services functions
EFI_ALLOCATE_PAGES          gStoredAllocatePages        = NULL;
EFI_ALLOCATE_POOL           gStoredAllocatePool         = NULL;
EFI_FREE_POOL               gStoredFreePool             = NULL;
EFI_GET_MEMORY_MAP          gStoredGetMemoryMap         = NULL;
EFI_EXIT_BOOT_SERVICES      gStoredExitBootServices     = NULL;
EFI_IMAGE_START             gStartImage                 = NULL;
EFI_HANDLE_PROTOCOL         gHandleProtocol             = NULL;
EFI_SET_VIRTUAL_ADDRESS_MAP gStoredSetVirtualAddressMap = NULL;
UINT32                      mOrgRTCRC32 = 0;

// monitoring AlocatePages
EFI_PHYSICAL_ADDRESS        gMinAllocatedAddr = 0;
EFI_PHYSICAL_ADDRESS        gMaxAllocatedAddr = 0;

// location of memory allocated by boot.efi for hibernate image
EFI_PHYSICAL_ADDRESS        gHibernateImageAddress = 0;

// last memory map obtained by boot.efi
UINTN                       gLastMemoryMapSize = 0;
EFI_MEMORY_DESCRIPTOR       *gLastMemoryMap = NULL;
UINTN                       gLastDescriptorSize = 0;
UINT32                      gLastDescriptorVersion = 0;

// saved exit boot services arguments
EFI_HANDLE                  gExitBSImageHandle = 0;
UINTN                       gExitBSMapKey       = 0; 

/** Helper function that calls GetMemoryMap() and returns new MapKey.
 */
EFI_STATUS
GetMemoryMapKey(
  OUT UINTN                   *MapKey,
  OUT EFI_MEMORY_DESCRIPTOR   **MemoryMap
  )
{
  UINTN                   MemoryMapSize;
  UINTN                   DescriptorSize;
  UINT32                  DescriptorVersion;

  return GetMemoryMapAlloc (NULL, &MemoryMapSize, MemoryMap, MapKey, &DescriptorSize, &DescriptorVersion);
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
  EFI_STATUS      res;
  EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput;

  // special handling if gEfiGraphicsOutputProtocolGuid is requested by boot.efi
  if (CompareGuid(Protocol, &gEfiGraphicsOutputProtocolGuid)) {
    res = gHandleProtocol(Handle, Protocol, Interface);
    if (res != EFI_SUCCESS) {
      // let's find it on some other handle
      res = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&GraphicsOutput);
      if (res == EFI_SUCCESS) {
        // return it
        *Interface = GraphicsOutput;
        //      DEBUG ((DEBUG_VERBOSE, "->HandleProtocol(%p, %s, %p) = %r (returning from other handle)\n", Handle, GuidStr(Protocol), *Interface, res));
        DEBUG ((DEBUG_VERBOSE, "->HandleProtocol(%p, %s, %p) = %r (from other handle)\n", Handle, GuidStr(Protocol), *Interface, res));
        return res;
      }
    }
    DEBUG ((DEBUG_VERBOSE, "->HandleProtocol(%p, %s, %p) = %r\n", Handle, GuidStr(Protocol), *Interface, res));
  } else {
    res = gHandleProtocol(Handle, Protocol, Interface);
  }
  //  DEBUG ((DEBUG_VERBOSE, "->HandleProtocol(%p, %s, %p) = %r\n", Handle, GuidStr(Protocol), *Interface, res));
  return res;
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
    // called from boot.efi

    UpperAddr = *Memory + EFI_PAGES_TO_SIZE(NumberOfPages);

    // store min and max mem - can be used later to determine start and end of kernel boot image
    if (gMinAllocatedAddr == 0 || *Memory < gMinAllocatedAddr)
      gMinAllocatedAddr = *Memory;
    if (UpperAddr > gMaxAllocatedAddr)
      gMaxAllocatedAddr = UpperAddr;

    Status = gStoredAllocatePages(Type, MemoryType, NumberOfPages, Memory);
  } else if (gHibernateWake && Type == AllocateAnyPages && MemoryType == EfiLoaderData) {
    // called from boot.efi during hibernate wake
    // first such allocation is for hibernate image
    Status = gStoredAllocatePages(Type, MemoryType, NumberOfPages, Memory);
    if (gHibernateImageAddress == 0 && Status == EFI_SUCCESS) {
      gHibernateImageAddress = *Memory;
    }
  } else {
    // default page allocation
    Status = gStoredAllocatePages(Type, MemoryType, NumberOfPages, Memory);
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
  return gStoredAllocatePool(Type, Size, Buffer);
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
  return gStoredFreePool(Buffer);
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

  Status = gStoredGetMemoryMap(MemoryMapSize, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
  DEBUG ((DEBUG_VERBOSE, "GetMemoryMap: %p = %r\n", MemoryMap, Status));
  if (Status == EFI_SUCCESS) {

#if APTIOFIX_ALLOW_MEMORY_DUMP_ARG == 1
    if (gDumpMemArgPresent) {
      Print (L"------------- GetMemoryMap update start -------------\n");
      Print (L"RtShims: %08lX, gST: %08lX\n",
        (UINTN)gRtShims, (UINTN)gSysTableRtArea);
      PrintMemMap (*MemoryMapSize, *DescriptorSize, MemoryMap);
      Print (L"-------------  GetMemoryMap update end  -------------\n");
      gBS->Stall (5000000);
    }
#endif

#if APTIOFIX_PROTECT_CSM_REGION == 1
    ProtectCsmRegion (*MemoryMapSize, MemoryMap, *DescriptorSize);
#endif

    ShrinkMemMap (MemoryMapSize, MemoryMap, *DescriptorSize);

    // remember last/final memmap
    gLastMemoryMapSize = *MemoryMapSize;
    gLastMemoryMap = MemoryMap;
    gLastDescriptorSize = *DescriptorSize;
    gLastDescriptorVersion = *DescriptorVersion;
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
  UINTN                    NewMapKey;
  EFI_MEMORY_DESCRIPTOR    *MemoryMap;
  UINTN                    SlideAddr = 0;
  VOID                     *MachOImage = NULL;
  IOHibernateImageHeader   *ImageHeader = NULL;

  // we need hibernate image address for wake
  if (gHibernateWake && gHibernateImageAddress == 0) {
    Print(L"AptioMemoryFix error: Doing hibernate wake, but did not find hibernate image address.\n");
    Print(L"... waiting 5 secs ...\n");
    gBS->Stall(5000000);
    return EFI_INVALID_PARAMETER;
  }

  // We can just return EFI_SUCCESS and continue using Print for debug.
#if APTIOFIX_ALLOW_MEMORY_DUMP_ARG == 1
  if (gDumpMemArgPresent) {
    gExitBSImageHandle = ImageHandle;
    gExitBSMapKey      = MapKey; 
    Status             = EFI_SUCCESS;
  } else
#endif
  {
    Status = gStoredExitBootServices(ImageHandle, MapKey);
  }

  DEBUG ((DEBUG_VERBOSE, "ExitBootServices:  = %r\n", Status));
  if (EFI_ERROR (Status)) {
    // just report error as var in nvram to be visible from macOS with "nvram -p"
    gRT->SetVariable(L"aptio-memfix-error-exitbs",
      &gAppleBootVariableGuid,
      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
      3,
      "Yes"
      );

    //Note: it is too late to free memory map here, but it does not matter,
    // because boot.efi has an old one and will freely use the memory.
    Status = GetMemoryMapKey(&NewMapKey, &MemoryMap);
    DEBUG ((DEBUG_VERBOSE, "ExitBootServices: GetMemoryMapKey = %r\n", Status));
    if (Status == EFI_SUCCESS) {
      // we have latest mem map and NewMapKey
      // we'll try again ExitBootServices with NewMapKey
      Status = gStoredExitBootServices(ImageHandle, NewMapKey);
      DEBUG ((DEBUG_VERBOSE, "ExitBootServices: 2nd try = %r\n", Status));
      if (EFI_ERROR (Status)) {
        // Error!
        Print(L"AptioMemoryFix: Error ExitBootServices() 2nd try = Status: %r\n", Status);
      }
    } else {
      Print(L"AptioMemoryFix: Error ExitBootServices(), GetMemoryMapKey() = Status: %r\n", Status);
      Status = EFI_INVALID_PARAMETER;
    }
  }

  if (EFI_ERROR(Status)) {
    Print(L"... waiting 10 secs ...\n");
    gBS->Stall(10*1000000);
    return Status;
  }

  if (!gHibernateWake) {
    // normal boot
    DEBUG ((DEBUG_VERBOSE, "ExitBootServices: gMinAllocatedAddr: %lx, gMaxAllocatedAddr: %lx\n", gMinAllocatedAddr, gMaxAllocatedAddr));

    SlideAddr = gMinAllocatedAddr - 0x100000;
    MachOImage = (VOID*)(UINTN)(SlideAddr + 0x200000);
    KernelEntryFromMachOPatchJump(MachOImage, SlideAddr);

  } else {
    // hibernate wake

    // at this stage HIB section is not yet copied from sleep image to it's
    // proper memory destination. so we'll patch entry point in sleep image.
    ImageHeader = (IOHibernateImageHeader *)(UINTN)gHibernateImageAddress;
    KernelEntryPatchJump( ((UINT32)(UINTN)&(ImageHeader->fileExtentMap[0])) + ImageHeader->fileExtentMapSize + ImageHeader->restore1CodeOffset );
  }

  return Status;
}

/** gRT->SetVirtualAddressMap override:
 * Fixes virtualizing of RT services.
 */
EFI_STATUS
EFIAPI
OvrSetVirtualAddressMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *VirtualMap
  )
{
  EFI_STATUS   Status;
  UINT32       EfiSystemTable;

  DEBUG ((DEBUG_VERBOSE, "->SetVirtualAddressMap(%d, %d, 0x%x, %p) START ...\n", MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap));

  // restore origs
  //FIXME: this crc is invalid
  gRT->Hdr.CRC32 = mOrgRTCRC32;
  gRT->SetVirtualAddressMap = gStoredSetVirtualAddressMap;

#if APTIOFIX_ALLOW_MEMORY_DUMP_ARG == 1
  if (gDumpMemArgPresent) {
    Print(L"------------- SetVirtualAddressMap update start -------------\n");
    Print(L"RtShims: %08lX, gST: %08lX, gRelST: %08X\n",
      (UINTN)gRtShims, (UINTN)gSysTableRtArea);
    PrintMemMap(MemoryMapSize, DescriptorSize, VirtualMap);
    Print(L"-------------  SetVirtualAddressMap update end  -------------\n");
    gBS->Stall(5000000);
    //
    // To print as much information as possible we delay ExitBootServices.
    // Most likely this will fail, but let's still try!
    //
    gStoredExitBootServices (gExitBSImageHandle, gExitBSMapKey);
    //TODO: fix map key on failure
  }
#endif

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

/** Callback called when boot.efi jumps to kernel. */
UINTN
EFIAPI
KernelEntryPatchJumpBack (
  UINTN    bootArgs,
  BOOLEAN  ModeX64
  )
{
  if (!gHibernateWake) {
    bootArgs = FixBootingWithoutRelocBlock(bootArgs, ModeX64);
  } else {
    bootArgs = FixHibernateWakeWithoutRelocBlock(bootArgs, ModeX64);
  }

  return bootArgs;
}

/** SWITCH_STACK_ENTRY_POINT implementation:
 * Allocates kernel image reloc block, installs UEFI overrides and starts given image.
 * If image returns, then deinstalls overrides and releases kernel image reloc block.
 *
 * If started with ImgContext->JumpBuffer, then it will return with LongJump().
 */
EFI_STATUS
RunImageWithOverrides(
  IN EFI_HANDLE                 ImageHandle,
  IN EFI_LOADED_IMAGE_PROTOCOL  *Image,
  OUT UINTN                     *ExitDataSize,
  OUT CHAR16                    **ExitData  OPTIONAL
  )
{
  EFI_STATUS           Status;

  //
  // Save current 64bit state - will be restored later in callback from kernel jump
  // and relocate JumpToKernel32 code to higher mem (for copying kernel back to
  // proper place and jumping back to it)
  //
  Status = PrepareJumpFromKernel();
  if (EFI_ERROR(Status)) {
    return Status;
  }

  // init VMem memory pool - will be used after ExitBootServices
  Status = VmAllocateMemoryPool();
  if (EFI_ERROR(Status)) {
    return Status;
  }

  InstallRtShims (GetVariableCustomSlide);

  // clear monitoring vars
  gMinAllocatedAddr = 0;
  gMaxAllocatedAddr = 0;

  // save original BS functions
  gStoredAllocatePages    = gBS->AllocatePages;
  //gStoredAllocatePool     = gBS->AllocatePool;
  //gStoredFreePool         = gBS->FreePool;
  gStoredGetMemoryMap     = gBS->GetMemoryMap;
  gStoredExitBootServices = gBS->ExitBootServices;
  gHandleProtocol         = gBS->HandleProtocol;

  // install our overrides
  gBS->AllocatePages    = MOAllocatePages;
  //gBS->AllocatePool     = MOAllocatePool;
  //gBS->FreePool         = MOFreePool;
  gBS->GetMemoryMap     = MOGetMemoryMap;
  gBS->ExitBootServices = MOExitBootServices;
  gBS->HandleProtocol   = MOHandleProtocol;

  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);

  mOrgRTCRC32 = gRT->Hdr.CRC32;
  gStoredSetVirtualAddressMap = gRT->SetVirtualAddressMap;
  gRT->SetVirtualAddressMap = OvrSetVirtualAddressMap;
  gRT->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);

  // force boot.efi to use our copy of system table
  DEBUG ((DEBUG_VERBOSE, "StartImage: orig sys table: %p\n", Image->SystemTable));
  Image->SystemTable = (EFI_SYSTEM_TABLE *)(UINTN)gSysTableRtArea;
  DEBUG ((DEBUG_VERBOSE, "StartImage: new sys table: %p\n", Image->SystemTable));

  ProcessBooterImage(ImageHandle);

  // run image
  Status = gStartImage(ImageHandle, ExitDataSize, ExitData);

  // if we get here then boot.efi did not start kernel
  // and we'll try to do some cleanup ...

  // return back originals
  gBS->AllocatePages    = gStoredAllocatePages;
  //gBS->AllocatePool     = gStoredAllocatePool;
  //gBS->FreePool         = gStoredFreePool;
  gBS->GetMemoryMap     = gStoredGetMemoryMap;
  gBS->ExitBootServices = gStoredExitBootServices;
  gBS->HandleProtocol   = gHandleProtocol;

  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);

  gRT->SetVirtualAddressMap = gStoredSetVirtualAddressMap;
  gBS->CalculateCrc32(gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);

  return Status;
}

/** gBS->StartImage override:
 * Called to start an efi image.
 *
 * If this is boot.efi, then run it with our overrides.
 */
EFI_STATUS
EFIAPI
MOStartImage (
  IN EFI_HANDLE      ImageHandle,
  OUT UINTN          *ExitDataSize,
  OUT CHAR16         **ExitData  OPTIONAL
  )
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *Image;
  CHAR16                      *FilePathText = NULL;
  UINTN                       Size          = 0;
  VOID                        *Value        = NULL;
  UINTN                       Size2         = 0;
  CHAR16                      *StartFlag    = NULL;

  DEBUG ((DEBUG_VERBOSE, "StartImage(%lx)\n", ImageHandle));

  // find out image name from EfiLoadedImageProtocol
  Status = gBS->OpenProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &Image, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_WARN, "ERROR: MOStartImage: OpenProtocol(gEfiLoadedImageProtocolGuid) = %r\n", Status));
    return EFI_INVALID_PARAMETER;
  }
  FilePathText = FileDevicePathToText(Image->FilePath);
  if (FilePathText != NULL) {
    DEBUG ((DEBUG_VERBOSE, "FilePath: %s\n", FilePathText));
  }
  DEBUG ((DEBUG_VERBOSE, "ImageBase: %p - %lx (%lx)\n", Image->ImageBase, (UINT64)Image->ImageBase + Image->ImageSize, Image->ImageSize));
  Status = gBS->CloseProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, gImageHandle, NULL);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_WARN, "CloseProtocol error: %r\n", Status));
  }

  if (StrStriBasic(FilePathText,L"boot.efi")){
    Status = GetVariable2 (L"aptiofixflag", &gAppleBootVariableGuid, &Value, &Size2);
    if (!EFI_ERROR(Status)) {
      Status = gRT->SetVariable(L"recovery-boot-mode", &gAppleBootVariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        Size2, Value);
      if (EFI_ERROR(Status)) {
        DEBUG ((DEBUG_WARN, "Something goes wrong while setting recovery-boot-mode\n"));
      }
      Status = gRT->SetVariable (L"aptiofixflag", &gAppleBootVariableGuid, 0, 0, NULL);
      DirectFreePool(Value);
    }

    Size2 =0;
    //Check recovery-boot-mode present for nested boot.efi
    Status = GetVariable2 (L"recovery-boot-mode", &gAppleBootVariableGuid, &Value, &Size2);
    if (!EFI_ERROR(Status)) {
      //If it presents, then wait for \com.apple.recovery.boot\boot.efi boot
      DEBUG ((DEBUG_VERBOSE, " recovery-boot-mode present\n"));
      StartFlag = StrStriBasic(FilePathText, L"\\com.apple.recovery.boot\\boot.efi");
      if (mCounter > 0x00){
        Status = gRT->SetVariable(L"aptiofixflag", &gAppleBootVariableGuid,
          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
          Size2, Value);
        if (EFI_ERROR(Status)) {
          DEBUG ((DEBUG_WARN, "Something goes wrong: %r\n", Status));
        }
        gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
      }
    } else {
      StartFlag = StrStriBasic(FilePathText, L"boot.efi");
    }
    DirectFreePool(Value);
  }

  // check if this is boot.efi
  // if (StrStriBasic(FilePathText, L"boot.efi")) {
  if (StartFlag) {
    mCounter++;
    //the presence of the variable means HibernateWake
    //if the wake is canceled then the variable must be deleted
    Status = gRT->GetVariable(L"boot-switch-vars", &gAppleBootVariableGuid, NULL, &Size, NULL);
    gHibernateWake = (Status == EFI_BUFFER_TOO_SMALL);

    Print(L"\nAptioMemoryFix(R%d): Starting %s\nHibernate wake: %s\n",
      mAptioMemoryFixProtocol.Revision,
      FilePathText,
      gHibernateWake ? L"yes" : L"no");
    //gBS->Stall(2000000);

    // run with our overrides
    Status = RunImageWithOverrides(ImageHandle, Image, ExitDataSize, ExitData);

  } else {
    // call original function to do the job
    Status = gStartImage(ImageHandle, ExitDataSize, ExitData);
  }

  if (FilePathText != NULL) {
    DirectFreePool(FilePathText);
  }
  return Status;
}

/** Entry point. Installs our StartImage override.
 * All other stuff will be installed from there when boot.efi is started.
 */
EFI_STATUS
EFIAPI
AptioMemoryFixEntrypoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS    Status;
  VOID          *Interface;
  EFI_HANDLE    Handle = NULL;

  Status = gBS->LocateProtocol (
                  &gAptioMemoryFixProtocolGuid,
                  NULL,
                  &Interface
                );

  if (!EFI_ERROR (Status)) {
    //
    // In case for whatever reason one tried to reload the driver.
    //
    return EFI_ALREADY_STARTED;
  }

  gBS->InstallProtocolInterface (
        &Handle,
        &gAptioMemoryFixProtocolGuid,
        EFI_NATIVE_INTERFACE,
        &mAptioMemoryFixProtocol
        );

  //
  // Install StartImage override
  // All other overrides will be started when boot.efi is started
  //
  gStartImage = gBS->StartImage;
  gBS->StartImage = MOStartImage;
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);

  return EFI_SUCCESS;
}
