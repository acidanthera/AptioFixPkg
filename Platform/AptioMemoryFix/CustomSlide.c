/**

  Provides fixes for 'slide'.

*/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/RngLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>

#include "Config.h"
#include "CustomSlide.h"
#include "Utils.h"
#include "BootArgs.h"
#include "Lib.h"
#include "FlatDevTree/device_tree.h"
#include "CsrConfig.h"
#include "RtShims.h"

// used for reporting boot-args with a custom slide
STATIC BOOLEAN gStoredBootArgsVarSet = FALSE;
STATIC BOOLEAN gAnalyzeMemoryMapDone = FALSE;
STATIC UINTN   gStoredBootArgsVarSize = 0;
STATIC CHAR8   gStoredBootArgsVar[BOOT_LINE_LENGTH] = {0};

// used for restoring csr-active-config in boot-args
STATIC BOOLEAN gCsrActiveConfigSet = FALSE;
STATIC UINT32  gCsrActiveConfig = 0;

// base kernel address and kaslr slide range
#define BASE_KERNEL_ADDR       ((UINTN)0x100000)
#define TOTAL_SLIDE_NUM        256

// user for custom aslr implimentation, when some values are not valid
STATIC UINT8   gValidSlides[TOTAL_SLIDE_NUM] = {0};
STATIC UINT32  gValidSlidesNum = TOTAL_SLIDE_NUM;

extern BOOLEAN gSlideArgPresent;

VOID
UnlockSlideSupportForSafeModeAndCheckSlide (
    UINT8 *ImageBase,
    UINTN ImageSize
)
{
  // boot.efi performs the following check:
  // if (State & (BOOT_MODE_SAFE | BOOT_MODE_ASLR)) == (BOOT_MODE_SAFE | BOOT_MODE_ASLR)) {
  //   * Disable KASLR *
  // }
  // We do not care about the asm it will use for it, but we could assume that the constants
  // will be used twice and very close to each other.

  // BOOT_MODE_SAFE | BOOT_MODE_ASLR constant is 0x4001 in hex.
  // It has not changed since its appearance, so is most likely safe to look for.
  // Furthermore, since boot.efi state mask uses higher bits, it is safe to assume that
  // the comparison will be at least 32-bit.
  CONST UINT8 SearchSeq[] = {0x01, 0x40, 0x00, 0x00};

  // This is a reasonable value to expect to be between the instructions.
  CONST UINTN MaxDist = 0x10;

  UINT8 *StartOff = ImageBase;
  UINT8 *EndOff   = StartOff + ImageSize - sizeof(SearchSeq) - MaxDist;

  UINTN FirstOff = 0;
  UINTN SecondOff = 0;

  do {
    while (
        StartOff + FirstOff <= EndOff &&
        CompareMem(StartOff + FirstOff, SearchSeq, sizeof(SearchSeq))
        ) {
      FirstOff++;
    }

    DEBUG ((DEBUG_VERBOSE, "Found first at off %X\n", (UINT32)FirstOff));

    if (StartOff + FirstOff > EndOff) {
      DEBUG ((DEBUG_WARN, "Failed to find first BOOT_MODE_SAFE | BOOT_MODE_ASLR sequence\n"));
      break;
    }

    SecondOff = FirstOff + sizeof(SearchSeq);

    while (
        StartOff + SecondOff <= EndOff && FirstOff + MaxDist >= SecondOff &&
        CompareMem(StartOff + SecondOff, SearchSeq, sizeof(SearchSeq))
        ) {
      SecondOff++;
    }

    DEBUG ((DEBUG_VERBOSE, "Found second at off %X\n", (UINT32)SecondOff));

    if (FirstOff + MaxDist < SecondOff) {
      DEBUG ((DEBUG_VERBOSE, "Trying next match...\n"));
      SecondOff = 0;
      FirstOff += sizeof(SearchSeq);
    }
  } while (SecondOff == 0);

  if (SecondOff != 0) {
    // Here we use 0xFFFFFFFF constant as a replacement value.
    // Since the state values are contradictive (e.g. safe & single at the same time)
    // We are allowed to use this instead of to simulate if (false).
    DEBUG ((DEBUG_VERBOSE, "Patching safe mode aslr check...\n"));
    SetMem(StartOff + FirstOff, sizeof(SearchSeq), 0xFF);
    SetMem(StartOff + SecondOff, sizeof(SearchSeq), 0xFF);
  }
}

STATIC
VOID
GetSlideRangeForValue (
    UINT8   Slide,
    UINTN   *StartAddr,
    UINTN   *EndAddr
)
{
  *StartAddr = (UINTN)Slide * 0x200000 + BASE_KERNEL_ADDR;

  // Skip ranges improperly used by Intel HD 2000/3000.
  if (Slide >= 0x80 && IsSandyOrIvy()) {
    *StartAddr += 0x10200000;
  }

  *EndAddr   = *StartAddr + APTIOFIX_SPECULATED_KERNEL_SIZE;
}

BOOLEAN
OverlapsWithSlide (
    EFI_PHYSICAL_ADDRESS   Address,
    UINTN                  Size
)
{
  BOOLEAN               SandyOrIvy;
  EFI_PHYSICAL_ADDRESS  Start;
  EFI_PHYSICAL_ADDRESS  End;
  UINTN                 Slide = 0xFF;

  SandyOrIvy = IsSandyOrIvy ();

  if (SandyOrIvy) {
    Slide = 0x7F;
  }

  Start = BASE_KERNEL_ADDR;
  End   = Start + Slide * 0x200000 + APTIOFIX_SPECULATED_KERNEL_SIZE;

  if (End >= Address && Start <= Address + Size) {
    return TRUE;
  } else if (SandyOrIvy) {
    Start = 0x80 * 0x200000 + BASE_KERNEL_ADDR + 0x10200000;
    End   = Start + Slide * 0x200000 + APTIOFIX_SPECULATED_KERNEL_SIZE;
    if (End >= Address && Start <= Address + Size) {
      return TRUE;
    }
  }

  return FALSE;
}

STATIC
UINT8
GenerateRandomSlideValue (
    VOID
)
{
  UINT32  Clock = 0;
  UINT32  Ecx = 0;
  UINT8   Slide = 0;
  UINT16  Value = 0;
  BOOLEAN RdRandSupport;

  AsmCpuid (0x1, NULL, NULL, &Ecx, NULL);
  RdRandSupport = (Ecx & 0x40000000) != 0;

  do {
    if (RdRandSupport &&
        GetRandomNumber16(&Value) == EFI_SUCCESS &&
        Slide != 0) {
      break;
    }

    Clock = (UINT32)AsmReadTsc();
    Slide = (Clock & 0xFF) ^ ((Clock >> 8) & 0xFF);
  } while (Slide == 0);

  //PrintScreen (L"Generated slide index %d value %d\n", Slide, gValidSlides[Slide % gValidSlidesNum]);

  //FIXME: This is bad due to uneven distribution, but let's use it for now.
  return gValidSlides[Slide % gValidSlidesNum];
}

STATIC
VOID
DecideOnCustomSlideImplementation (
    VOID
)
{
  UINTN                  AllocatedMapPages;
  UINTN                  MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR  *MemoryMap;
  UINTN                  MapKey;
  EFI_STATUS             Status;
  UINTN                  DescriptorSize;
  UINT32                 DescriptorVersion;
  UINTN                  Index;
  UINTN                  Slide;
  UINTN                  NumEntries;

  Status = GetMemoryMapAlloc (
      &AllocatedMapPages,
      &MemoryMapSize,
      &MemoryMap,
      &MapKey,
      &DescriptorSize,
      &DescriptorVersion
  );

  if (Status != EFI_SUCCESS) {
    PrintScreen (L"AMF: Failed to obtain memory map for KASLR - %r\n", Status);
    return;
  }

  // At this point we have a memory map that we could use to determine what slide values are allowed.
  NumEntries = MemoryMapSize / DescriptorSize;

  // Reset valid slides to zero and find actually working ones.
  gValidSlidesNum = 0;

  for (Slide = 0; Slide < TOTAL_SLIDE_NUM; Slide++) {
    EFI_MEMORY_DESCRIPTOR  *Desc = MemoryMap;
    BOOLEAN                Supported = TRUE;
    UINTN                  StartAddr;
    UINTN                  EndAddr;
    UINTN                  DescEndAddr;
    UINTN                  AvailableSize;

    GetSlideRangeForValue ((UINT8)Slide, &StartAddr, &EndAddr);

    AvailableSize = 0;

    for (Index = 0; Index < NumEntries; Index++) {
      DescEndAddr = (Desc->PhysicalStart + EFI_PAGES_TO_SIZE (Desc->NumberOfPages));

      if ((Desc->PhysicalStart < EndAddr) && (DescEndAddr > StartAddr)) {
        //
        // The memory overlaps with the slide region.
        //
        if (Desc->Type != EfiConventionalMemory) {
          //
          // The memory is unusable atm.
          //
          Supported = FALSE;
          break;
        } else {
          //
          // The memory will be available for the kernel.
          //
          AvailableSize += EFI_PAGES_TO_SIZE (Desc->NumberOfPages);

          if (Desc->PhysicalStart < StartAddr) {
            //
            // The region starts before the slide region.  Subtract the memory
            // that is located before the slide region.
            //
            AvailableSize -= (StartAddr - Desc->PhysicalStart);
          }

          if (DescEndAddr > EndAddr) {
            //
            // The region ends after the slide region.  Subtract the memory
            // that is located after the slide region.
            //
            AvailableSize -= (DescEndAddr - EndAddr);
          }
        }
      }

      Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
    }

    if ((StartAddr + AvailableSize) != EndAddr) {
      //
      // The slide region is not continuous.
      //
      Supported = FALSE;
    }

    if (Supported) {
      DEBUG ((DEBUG_VERBOSE, "Slide %03d at %08x:%08x should be ok.\n", (UINT32)Slide, (UINT32)StartAddr, (UINT32)EndAddr));
      gValidSlides[gValidSlidesNum++] = (UINT8)Slide;
    } else {
      DEBUG ((DEBUG_VERBOSE, "Slide %03d at %08x:%08x cannot be used!\n", (UINT32)Slide, (UINT32)StartAddr, (UINT32)EndAddr));
    }
  }

  gBS->FreePages ((EFI_PHYSICAL_ADDRESS)MemoryMap, AllocatedMapPages);

  if (gValidSlidesNum != TOTAL_SLIDE_NUM) {
    if (gValidSlidesNum == 0) {
      PrintScreen (L"AMF: No slide values are usable! Use custom slide!\n");
    } else {
      //
      // Pretty-print valid slides as ranges.
      // For example, 1, 2, 3, 4, 5 will becomes 1-5.
      //
      PrintScreen (L"AMF: Only %d/%d slide values are usable! Booting may fail!\n", gValidSlidesNum, TOTAL_SLIDE_NUM);
      NumEntries = 0;
      for (Index = 0; Index <= gValidSlidesNum; Index++) {
        if (Index == 0) {
          PrintScreen (L"Valid slides: %d", gValidSlides[Index]);
        } else if (Index == gValidSlidesNum || gValidSlides[Index - 1] + 1 != gValidSlides[Index]) {
          if (NumEntries == 1) {
            PrintScreen(L", %d", gValidSlides[Index - 1]);
          } else if (NumEntries > 1) {
            PrintScreen(L"-%d", gValidSlides[Index - 1]);
          }
          if (Index == gValidSlidesNum) {
            PrintScreen(L"\n");
          } else {
            PrintScreen(L", %d", gValidSlides[Index]);
          }
          NumEntries = 0;
        } else {
          NumEntries++;
        }
      }
    }
  }
}

EFI_STATUS
EFIAPI
GetVariableCustomSlide (
  CHAR16                  *VariableName,
  EFI_GUID                *VendorGuid,
  UINT32                  *Attributes,
  UINTN                   *DataSize,
  VOID                    *Data
  )
{
  EFI_GET_VARIABLE RealGetVariable = (EFI_GET_VARIABLE)gGetVariable;
  EFI_STATUS       Status;

  // We override csr-active-config with CSR_ALLOW_UNRESTRICTED_NVRAM bit set
  // to allow one to pass a custom slide value even when SIP is on.
  // This original value of csr-active-config is returned to OS at XNU boot.
  // This allows SIP to be fully enabled in the operating system.
  BOOLEAN          IsCsrActiveConfig = FALSE;

  // When we cannot allow some KASLR values due to used address we generate
  // a random slide value among the valid options, which we we pass via boot-args.
  // See DecideOnCustomSlideImplementation for more details.
  BOOLEAN          IsBootArgs = FALSE;

  // Basic checks
  if (VariableName && VendorGuid && DataSize &&
    !CompareMem (VendorGuid, &gAppleBootVariableGuid, sizeof(EFI_GUID))) {
    if (!StrCmp (VariableName, L"csr-active-config")) {
      // If we ask for the size, give it to it
      if (!Data) {
        *DataSize = 4;
        return EFI_BUFFER_TOO_SMALL;
      }
      // Otherwise pass our values
      IsCsrActiveConfig = TRUE;
    }
#if APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION == 1
    else if (!StrCmp (VariableName, L"boot-args")) {
      //
      // We delay memory map analysis as much as we can, in case boot.efi or anything else allocate
      // stuff with gBS->AllocatePool and this is not caught by our gBS->AllocatePool override.
      // This is a problem when an allocated region overlaps with the slide region (e.g. on X299).
      //
      if (!gSlideArgPresent && !gAnalyzeMemoryMapDone) {
        DecideOnCustomSlideImplementation();
        gAnalyzeMemoryMapDone = TRUE;
      }

      if (gValidSlidesNum != TOTAL_SLIDE_NUM && gValidSlidesNum > 0) {
        //
        // Only return custom boot-args if gValidSlidesNum were determined to be less than TOTAL_SLIDE_NUM
        // And thus we have to use a custom slide implementation to boot reliably.
        //
        IsBootArgs = TRUE;
      }
    }
#endif
  }

  if (IsCsrActiveConfig) {
    Status = RealGetVariable (VariableName, VendorGuid, Attributes, DataSize, Data);
    UINT32 *Config = (UINT32 *)Data;
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "GetVariable csr-active-config returned %r\n", Status));
      *Config = 0;
      Status = EFI_SUCCESS;
      if (Attributes) {
        *Attributes =
          EFI_VARIABLE_BOOTSERVICE_ACCESS |
          EFI_VARIABLE_RUNTIME_ACCESS |
          EFI_VARIABLE_NON_VOLATILE;
      }
    }
    // We must unrestrict NVRAM from SIP or slide=X will not be supported.
    gCsrActiveConfig     = *Config;
    gCsrActiveConfigSet  = TRUE;
    *Config |= CSR_ALLOW_UNRESTRICTED_NVRAM;
  } else if (IsBootArgs) {
    if (!gStoredBootArgsVarSet) {
      UINTN StoredBootArgsSize = BOOT_LINE_LENGTH;
      UINT8 Slide = GenerateRandomSlideValue ();
      Status = RealGetVariable (VariableName, VendorGuid, Attributes, &StoredBootArgsSize, gStoredBootArgsVar);

      CHAR8 *AppendPtr = gStoredBootArgsVar;
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_WARN, "boot-args returned %r error\n", Status));
      } else {
        UINTN Len = AsciiStrLen(gStoredBootArgsVar);
        if (Len + ARRAY_SIZE(" slide=123") > BOOT_LINE_LENGTH) {
          DEBUG ((DEBUG_WARN, "boot-args are invalid, ignoring\n"));
        } else {
          AppendPtr    += Len;
          *AppendPtr++ = ' ';
        }
      }

      CONST UINTN SlideStrLen = ARRAY_SIZE("slide=") - 1;
      AsciiStrnCpyS(AppendPtr, SlideStrLen + 1, "slide=", SlideStrLen + 1);
      UINT8 First  = Slide / 100;
      UINT8 Second = (Slide % 100) / 10;
      UINT8 Third  = Slide % 10;

      if (Slide < 100) {
        if (Slide >= 10) {
          First  = Second;
          Second = Third;
          Third  = DEC_SPACE;
        } else {
          First  = Third;
          Second = DEC_SPACE;
          Third  = DEC_SPACE;
        }
      }

      // Note, the point is to always pass 3 characters to avoid side attacks on value length.
      AppendPtr[SlideStrLen + 0] = DEC_TO_ASCII(First);
      AppendPtr[SlideStrLen + 1] = DEC_TO_ASCII(Second);
      AppendPtr[SlideStrLen + 2] = DEC_TO_ASCII(Third);
      AppendPtr[SlideStrLen + 3] = '\0';

      gStoredBootArgsVarSize = AsciiStrLen(gStoredBootArgsVar) + 1;
      gStoredBootArgsVarSet = TRUE;
    }

    if (Attributes) {
      *Attributes =
        EFI_VARIABLE_BOOTSERVICE_ACCESS |
        EFI_VARIABLE_RUNTIME_ACCESS |
        EFI_VARIABLE_NON_VOLATILE;
    }

    if (*DataSize >= gStoredBootArgsVarSize && Data) {
      AsciiStrnCpyS(Data, *DataSize, gStoredBootArgsVar, gStoredBootArgsVarSize);
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_BUFFER_TOO_SMALL;
    }

    *DataSize = gStoredBootArgsVarSize;
  } else {
    Status = RealGetVariable (VariableName, VendorGuid, Attributes, DataSize, Data);
  }

  return Status;
}

VOID
HideSlideFromOS (
    AMF_BOOT_ARGUMENTS *BootArgs
)
{
  DTEntry     DevTree;
  DTEntry     Chosen;
  CHAR8       *ArgsStr;
  UINTN       ArgsSize;

  // First, there is a BootArgs entry for XNU
  RemoveArgumentFromCommandLine(BootArgs->CommandLine, "slide=");

  // Second, there is a DT entry
  DevTree = (DTEntry)(UINTN)(*BootArgs->deviceTreeP);

  DTInit(DevTree);
  if (DTLookupEntry(NULL, "/chosen", &Chosen) == kSuccess) {
    DEBUG ((DEBUG_VERBOSE, "Found /chosen\n"));
    if (DTGetProperty(Chosen, "boot-args", (VOID **)&ArgsStr, &ArgsSize) == kSuccess && ArgsSize > 0) {
      DEBUG ((DEBUG_VERBOSE, "Found boot-args in /chosen\n"));
      RemoveArgumentFromCommandLine(ArgsStr, "slide=");
    }
  }

  // Third, clean the boot args just in case
  gValidSlidesNum = 0;
  gStoredBootArgsVarSize = 0;
  ZeroMem(gValidSlides, sizeof(gValidSlides));
  ZeroMem(gStoredBootArgsVar, sizeof(gStoredBootArgsVar));
}

VOID
FixBootingForCustomSlide(
    AMF_BOOT_ARGUMENTS *BA
)
{
  // Restore csr-active-config to a value it was before our slide=X alteration.
  if (BA->csrActiveConfig && gCsrActiveConfigSet) {
    *BA->csrActiveConfig = gCsrActiveConfig;
  }

#if APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT == 1
  // Having slide=X values visible in the operating system defeats the purpose of KASLR.
  // Since our custom implementation works by passing random KASLR slide via boot-args,
  // this is especially important.
  HideSlideFromOS(BA);
#endif
}
