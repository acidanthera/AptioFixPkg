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
#include "Hibernate.h"
#include "RtShims.h"
#include "CustomSlide.h"
#include "ServiceOverrides.h"

//
// One could discover AptioMemoryFix with this protocol
//
STATIC APTIOMEMORYFIX_PROTOCOL mAptioMemoryFixProtocol = {
  APTIOMEMORYFIX_PROTOCOL_REVISION
};

//
// Amount of nested boot.efi detected
//
STATIC UINTN            mBootNestedCount = 0;

//
// Original gBS->StartImage
//
STATIC EFI_IMAGE_START  mStartImage = NULL;

/** Callback called when boot.efi jumps to kernel. */
UINTN
EFIAPI
KernelEntryPatchJumpBack (
  UINTN    Args,
  BOOLEAN  ModeX64
  )
{
  if (!gHibernateWake)
    Args = FixBooting (Args);
  else
    Args = FixHibernateWake (Args);

  return Args;
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
  Status = PrepareJumpFromKernel ();
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Init VMem memory pool - will be used after ExitBootServices
  //
  Status = VmAllocateMemoryPool ();
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Install permanent shims and temporary overrides
  //
  InstallRtShims (GetVariableCustomSlide);
  InstallBsOverrides ();
  InstallRtOverrides ();

  //
  // Clear monitoring vars
  //
  gMinAllocatedAddr = 0;
  gMaxAllocatedAddr = 0;

  //
  // Force boot.efi to use our copy of system table
  //
  DEBUG ((DEBUG_VERBOSE, "StartImage: orig sys table: %p\n", Image->SystemTable));
  Image->SystemTable = (EFI_SYSTEM_TABLE *)(UINTN)gSysTableRtArea;
  DEBUG ((DEBUG_VERBOSE, "StartImage: new sys table: %p\n", Image->SystemTable));

#if APTIOFIX_ALLOW_ASLR_IN_SAFE_MODE == 1
  UnlockSlideSupportForSafeMode ((UINT8 *)Image->ImageBase, Image->ImageSize);
#endif

  //
  // Read options
  //
  ReadBooterArguments((CHAR16*)Image->LoadOptions, Image->LoadOptionsSize/sizeof(CHAR16));

  //
  // Run image
  //
  Status = mStartImage (ImageHandle, ExitDataSize, ExitData);

  //
  // If we get here then boot.efi did not start kernel
  // and we'll try to do some cleanup ...
  //
  UninstallBsOverrides ();
  UninstallRtOverrides ();
  UninstallRtShims ();

  return Status;
}

/** gBS->StartImage override:
 * Called to start an efi image.
 *
 * If this is boot.efi, then run it with our overrides.
 */
EFI_STATUS
EFIAPI
DetectBooterStartImage (
  IN EFI_HANDLE      ImageHandle,
  OUT UINTN          *ExitDataSize,
  OUT CHAR16         **ExitData  OPTIONAL
  )
{
  EFI_STATUS                  Status;
  CHAR16                      *FilePathText = NULL;
  VOID                        *Value        = NULL;
  EFI_LOADED_IMAGE_PROTOCOL   *LoadedImage = NULL;
  UINTN                       ValueSize     = 0;

  DEBUG ((DEBUG_VERBOSE, "StartImage (%lx)\n", ImageHandle));

  //
  // Find out image name from EfiLoadedImageProtocol
  //
  Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "StartImage: OpenProtocol(gEfiLoadedImageProtocolGuid) = %r\n", Status));
    return EFI_INVALID_PARAMETER;
  }

  FilePathText = FileDevicePathToStr (LoadedImage->FilePath);
  DEBUG ((DEBUG_VERBOSE, "ImageBase: %p - %lx (%lx)\n",
      LoadedImage->ImageBase,
      (UINT64)LoadedImage->ImageBase + LoadedImage->ImageSize,
      LoadedImage->ImageSize
    ));
  DEBUG ((DEBUG_VERBOSE, "FilePath: %s\n", FilePathText ? FilePathText : L"(Unknown)"));

  if (FilePathText && StrStriBasic (FilePathText, L"boot.efi")) {
    //
    // Chainloading boot.efi is currently unsupported (TODO: explore this).
    // The code below informs the bootloader about requested recovery mode.
    // This allows the bootloader to load the right boot.efi after a reboot and avoid chainloading.
    // Do not change this variable without bootloader changes!
    //
    Status = GetVariable2 (L"aptiofixflag", &gAppleBootVariableGuid, &Value, &ValueSize);
    if (!EFI_ERROR (Status)) {
      Status = gRT->SetVariable (L"recovery-boot-mode", &gAppleBootVariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        ValueSize, Value);

      if (EFI_ERROR (Status))
        DEBUG ((DEBUG_WARN, "Failed to set recovery-boot-mode: %r\n", Status));

      Status = gRT->SetVariable (L"aptiofixflag", &gAppleBootVariableGuid, 0, 0, NULL);
      FreePool (Value);
      ValueSize = 0;
    }

    //
    // Check recovery-boot-mode present for nested boot.efi
    //
    Status = GetVariable2 (L"recovery-boot-mode", &gAppleBootVariableGuid, &Value, &ValueSize);
    if (!EFI_ERROR (Status)) {
      DEBUG ((DEBUG_VERBOSE, "Discovered recovery-boot-mode\n"));

      //
      // This is an attempt to load a nested boot.efi, which is unsupported.
      // Save a special variable and let the bootloader load the right boot.efi.
      //
      if (mBootNestedCount > 0) {
        Status = gRT->SetVariable (L"aptiofixflag", &gAppleBootVariableGuid,
          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
          ValueSize, Value);

        if (EFI_ERROR (Status))
          DEBUG ((DEBUG_WARN, "Failed to set aptiofixflag: %r\n", Status));

        //
        // Here we do cold reset, because warm was causing issues on older motherboards.
        // Furthermore, it may not be supported
        //
        gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
      }

      FreePool (Value);
    }

    mBootNestedCount++;

    //
    // The presence of the variable means HibernateWake
    // To cancel hibernate wake it is enough to delete the variable
    //
    Status = gRT->GetVariable (L"boot-switch-vars", &gAppleBootVariableGuid, NULL, &ValueSize, NULL);
    gHibernateWake = Status == EFI_BUFFER_TOO_SMALL;

    PrintScreen (L"\nAptioMemoryFix(R%d): Starting %s%s\n",
      mAptioMemoryFixProtocol.Revision,
      FilePathText,
      gHibernateWake ? L" (hibernate wake)" : L"");

    FreePool (FilePathText);

    //
    // Run boot.efi with our overrides
    //
    Status = RunImageWithOverrides (ImageHandle, LoadedImage, ExitDataSize, ExitData);
  } else {
    //
    // Call the original function to do the job for any other booter
    //
    Status = mStartImage (ImageHandle, ExitDataSize, ExitData);
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
  mStartImage = gBS->StartImage;
  gBS->StartImage = DetectBooterStartImage;
  gBS->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gBS, gBS->Hdr.HeaderSize, &gBS->Hdr.CRC32);

  return EFI_SUCCESS;
}
