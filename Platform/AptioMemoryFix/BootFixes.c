/**

  Methods for setting callback jump from kernel entry point, callback, fixes to kernel boot image.

  by dmazar

**/

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/RngLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/LoadedImage.h>

#include "Config.h"
#include "BootArgs.h"
#include "BootFixes.h"
#include "AsmFuncs.h"
#include "VMem.h"
#include "Lib.h"
#include "Mach-O/Mach-O.h"
#include "FlatDevTree/device_tree.h"
#include "CsrConfig.h"
#include "Hibernate.h"
#include "RtShims.h"

// buffer and size for original kernel entry code
UINT8 gOrigKernelCode[32];
UINTN gOrigKernelCodeSize = 0;

// buffer for virtual address map - only for RT areas
// note: DescriptorSize is usually > sizeof(EFI_MEMORY_DESCRIPTOR)
// so this buffer can hold less then 64 descriptors
EFI_MEMORY_DESCRIPTOR gVirtualMemoryMap[64];
UINTN gVirtualMapSize = 0;
UINTN gVirtualMapDescriptorSize = 0;

// XNU requires gST pointers to be passed relative to boot.efi.
// We have to allocate a new system table and let boot.efi relocate it.
EFI_PHYSICAL_ADDRESS gSysTableRtArea = 0;
EFI_PHYSICAL_ADDRESS gRelocatedSysTableRtArea = 0;

RT_RELOC_PROTECT_DATA gRelocInfoData;

// used for restoring csr-active-config in boot-args
BOOLEAN gCsrActiveConfigSet = FALSE;
UINT32  gCsrActiveConfig = 0;

// used for reporting boot-args with a custom slide
BOOLEAN gStoredBootArgsVarSet = FALSE;
BOOLEAN gAnalyzeMemoryMapDone = FALSE;
UINTN   gStoredBootArgsVarSize = 0;
CHAR8   gStoredBootArgsVar[BOOT_LINE_LENGTH] = {0};

// base kernel address and kaslr slide range
#define BASE_KERNEL_ADDR       ((UINTN)0x100000)
#define TOTAL_SLIDE_NUM        256

// user for custom aslr implimentation, when some values are not valid
UINT8   gValidSlides[TOTAL_SLIDE_NUM] = {0};
UINT32  gValidSlidesNum = TOTAL_SLIDE_NUM;

// slide calculation on Sandy and Ivy Bridge CPUs needs special treatment
BOOLEAN gSandyOrIvy = FALSE;
BOOLEAN gSandyOrIvySet = FALSE;

void PrintSample2(unsigned char *sample, int size) {
  int i;
  for (i = 0; i < size; i++) {
    DEBUG ((DEBUG_VERBOSE, " %02x", *sample));
    sample++;
  }
}

//
// Kernel entry patching
//

/** Saves current 64 bit state and copies JumpToKernel32 function to higher mem
  * (for copying kernel back to proper place and jumping back to it).
  */
EFI_STATUS
PrepareJumpFromKernel (
  VOID
  )
{
  EFI_STATUS              Status;
  EFI_PHYSICAL_ADDRESS    HigherMem;
  UINTN                   Size;

  //
  // chek if already prepared
  //
  if (JumpToKernel32Addr != 0) {
    DEBUG ((DEBUG_VERBOSE, "PrepareJumpFromKernel() - already prepared\n"));
    return EFI_SUCCESS;
  }

  //
  // save current 64bit state - will be restored later in callback from kernel jump
  //
  AsmPrepareJumpFromKernel();

  //
  // allocate higher memory for JumpToKernel code
  //
  HigherMem = BASE_4GB;
  Status = AllocatePagesFromTop(EfiBootServicesCode, 1, &HigherMem);
  if (Status != EFI_SUCCESS) {
    Print(L"AptioMemoryFix: PrepareJumpFromKernel(): can not allocate mem for JumpToKernel (0x%x pages on mem top): %r\n",
      1, Status);
    return Status;
  }

  //
  // and relocate it to higher mem
  //
  JumpToKernel32Addr = HigherMem + ( (UINT8*)(UINTN)&JumpToKernel32 - (UINT8*)(UINTN)&JumpToKernel );
  JumpToKernel64Addr = HigherMem + ( (UINT8*)(UINTN)&JumpToKernel64 - (UINT8*)(UINTN)&JumpToKernel );

  Size = (UINT8*)&JumpToKernelEnd - (UINT8*)&JumpToKernel;
  if (Size > EFI_PAGES_TO_SIZE(1)) {
    Print(L"Size of JumpToKernel32 code is too big\n");
    return EFI_BUFFER_TOO_SMALL;
  }

  CopyMem((VOID *)(UINTN)HigherMem, (VOID *)&JumpToKernel, Size);

  DEBUG ((DEBUG_VERBOSE, "PrepareJumpFromKernel(): JumpToKernel relocated from %p, to %x, size = %x\n",
    &JumpToKernel, HigherMem, Size));
  DEBUG ((DEBUG_VERBOSE, " JumpToKernel32 relocated from %p, to %x\n",
    &JumpToKernel32, JumpToKernel32Addr));
  DEBUG ((DEBUG_VERBOSE, " JumpToKernel64 relocated from %p, to %x\n",
    &JumpToKernel64, JumpToKernel64Addr));
  DEBUG ((DEBUG_VERBOSE, "SavedCR3 = %x, SavedGDTR = %x, SavedIDTR = %x\n", SavedCR3, SavedGDTR, SavedIDTR));

  DEBUG ((DEBUG_VERBOSE, "PrepareJumpFromKernel(): JumpToKernel relocated from %p, to %x, size = %x\n",
    &JumpToKernel, HigherMem, Size));

  // Allocate 1 RT data page for copy of EFI system table for kernel
  gSysTableRtArea = BASE_4GB;
  Status = AllocatePagesFromTop(EfiRuntimeServicesData, 1, &gSysTableRtArea);
  if (Status != EFI_SUCCESS) {
    Print(L"AptioMemoryFix: PrepareJumpFromKernel(): can not allocate mem for RT area for EFI system table: %r\n",
      1, Status);
    return Status;
  }
  DEBUG ((DEBUG_VERBOSE, "gSysTableRtArea = %lx\n", gSysTableRtArea));

  // Copy sys table to our location
  EFI_SYSTEM_TABLE *Src  = (EFI_SYSTEM_TABLE*)(UINTN)gST;
  EFI_SYSTEM_TABLE *Dest = (EFI_SYSTEM_TABLE*)(UINTN)gSysTableRtArea;
  DEBUG ((DEBUG_VERBOSE, "-Copy %p <- %p, size=0x%lx\n", Dest, Src, Src->Hdr.HeaderSize));
  CopyMem(Dest, Src, Src->Hdr.HeaderSize);

  return Status;
}

/** Patches kernel entry point with jump to AsmJumpFromKernel (AsmFuncsX64). This will then call KernelEntryPatchJumpBack. */
EFI_STATUS
KernelEntryPatchJump (
  UINT32 KernelEntry
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_VERBOSE, "KernelEntryPatchJump KernelEntry (reloc): %lx (%lx)\n", KernelEntry, KernelEntry));

  // Size of EntryPatchCode code
  gOrigKernelCodeSize = (UINT8*)&EntryPatchCodeEnd - (UINT8*)&EntryPatchCode;
  if (gOrigKernelCodeSize > sizeof(gOrigKernelCode)) {
    DEBUG ((DEBUG_WARN, "KernelEntryPatchJump: not enough space for orig. kernel entry code: size needed: %d\n", gOrigKernelCodeSize));
    return EFI_NOT_FOUND;
  }

  DEBUG ((DEBUG_VERBOSE, "EntryPatchCode: %p, Size: %d, AsmJumpFromKernel: %p\n", &EntryPatchCode, gOrigKernelCodeSize, &AsmJumpFromKernel));

  // Save original kernel entry code
  CopyMem((VOID *)gOrigKernelCode, (VOID *)(UINTN)KernelEntry, gOrigKernelCodeSize);

  // Copy EntryPatchCode code to kernel entry address
  CopyMem((VOID *)(UINTN)KernelEntry, (VOID *)&EntryPatchCode, gOrigKernelCodeSize);

  DEBUG ((DEBUG_VERBOSE, "Entry point %x is now: ", KernelEntry));
  PrintSample2((UINT8 *)(UINTN) KernelEntry, 12);
  DEBUG ((DEBUG_VERBOSE, "\n"));

  // pass KernelEntry to assembler funcs
  // this is not needed really, since asm code will determine
  // kernel entry address from the stack
  AsmKernelEntry = KernelEntry;

  return Status;
}

/** Reads kernel entry from Mach-O load command and patches it with jump to AsmJumpFromKernel. */
EFI_STATUS
KernelEntryFromMachOPatchJump(VOID *MachOImage, UINTN SlideAddr)
{
  UINTN  KernelEntry;

  DEBUG ((DEBUG_VERBOSE, "KernelEntryFromMachOPatchJump: MachOImage = %p, SlideAddr = %x\n", MachOImage, SlideAddr));

  KernelEntry = MachOGetEntryAddress(MachOImage);
  DEBUG ((DEBUG_VERBOSE, "KernelEntryFromMachOPatchJump: KernelEntry = %x\n", KernelEntry));

  if (KernelEntry == 0) {
    return EFI_NOT_FOUND;
  }

  if (SlideAddr > 0) {
    KernelEntry += SlideAddr;
    DEBUG ((DEBUG_VERBOSE, "KernelEntryFromMachOPatchJump: Slided KernelEntry = %x\n", KernelEntry));
  }

  return KernelEntryPatchJump((UINT32)KernelEntry);
}

//
// Boot fixes
//

/** Copies RT flagged areas to separate memmap, defines virtual to phisycal address mapping
 * and calls SetVirtualAddressMap() only with that partial memmap.
 *
 * About partial memmap:
 * Some UEFIs are converting pointers to virtual addresses even if they do not
 * point to regions with RT flag. This means that those UEFIs are using
 * Desc->VirtualStart even for non-RT regions. Linux had issues with this:
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=7cb00b72876ea2451eb79d468da0e8fb9134aa8a
 * They are doing it Windows way now - copying RT descriptors to separate
 * mem map and passing that stripped map to SetVirtualAddressMap().
 * We'll do the same, although it seems that just assigning
 * VirtualStart = PhysicalStart for non-RT areas also does the job.
 *
 * About virtual to phisycal mappings:
 * Also adds virtual to phisycal address mappings for RT areas. This is needed since
 * SetVirtualAddressMap() does not work on my Aptio without that. Probably because some driver
 * has a bug and is trying to access new virtual addresses during the change.
 * Linux and Windows are doing the same thing and problem is
 * not visible there.
 */
EFI_STATUS
ExecSetVirtualAddressesToMemMap (
  IN UINTN                    MemoryMapSize,
  IN UINTN                    DescriptorSize,
  IN UINT32                   DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR    *MemoryMap
  )
{
  UINTN                           NumEntries;
  UINTN                           Index;
  EFI_MEMORY_DESCRIPTOR           *Desc;
  EFI_MEMORY_DESCRIPTOR           *VirtualDesc;
  EFI_STATUS                      Status;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PageTable;
  UINTN                           Flags;
  UINTN                           BlockSize;

  Desc                      = MemoryMap;
  NumEntries                = MemoryMapSize / DescriptorSize;
  VirtualDesc               = gVirtualMemoryMap;
  gVirtualMapSize           = 0;
  gVirtualMapDescriptorSize = DescriptorSize;
  DEBUG ((DEBUG_VERBOSE, "ExecSetVirtualAddressesToMemMap: Size=%d, Addr=%p, DescSize=%d\n", MemoryMapSize, MemoryMap, DescriptorSize));

  // get current VM page table
  GetCurrentPageTable (&PageTable, &Flags);

  for (Index = 0; Index < NumEntries; Index++) {
    //
    // Some UEFIs end up with "reserved" area with EFI_MEMORY_RUNTIME flag set when Intel HD3000 or HD4000 is used.
    // For example, on GA-H81N-D2H there is a single 1 GB descriptor:
    // 000000009F800000-00000000DF9FFFFF 0000000000040200 8000000000000000
    //
    // All known boot.efi starting from at least 10.5.8 properly handle this flag and do not assign virtual addresses
    // to reserved descriptors.
    // However, the issue was with AptioFix itself, which did not check for EfiReservedMemoryType and replaced
    // it by EfiMemoryMappedIO to prevent boot.efi relocations.
    //
    // The relevant discussion and the original fix can be found here:
    // http://web.archive.org/web/20141111124211/http://www.projectosx.com:80/forum/lofiversion/index.php/t2428-450.html
    // https://sourceforge.net/p/cloverefiboot/code/605/
    //
    // Since it is not the bug in boot.efi, AptioMemoryFix only needs to properly handle EfiReservedMemoryType with
    // EFI_MEMORY_RUNTIME attribute set, and there is no reason to mess with the memory map passed to boot.efi.
    //
    if (Desc->Type != EfiReservedMemoryType && (Desc->Attribute & EFI_MEMORY_RUNTIME) != 0) {
      // check if there is enough space in gVirtualMemoryMap
      if (gVirtualMapSize + DescriptorSize > sizeof(gVirtualMemoryMap)) {
        DEBUG ((DEBUG_WARN, "ERROR: too much mem map RT areas\n"));
        return EFI_OUT_OF_RESOURCES;
      }

      // copy region with EFI_MEMORY_RUNTIME flag to gVirtualMemoryMap
      CopyMem ((VOID*)VirtualDesc, (VOID*)Desc, DescriptorSize);

      // define virtual to phisical mapping
      DEBUG ((DEBUG_VERBOSE, "Map pages: %lx (%x) -> %lx\n", Desc->VirtualStart, Desc->NumberOfPages, Desc->PhysicalStart));
      VmMapVirtualPages (PageTable, Desc->VirtualStart, Desc->NumberOfPages, Desc->PhysicalStart);

      // next gVirtualMemoryMap slot
      VirtualDesc = NEXT_MEMORY_DESCRIPTOR (VirtualDesc, DescriptorSize);
      gVirtualMapSize += DescriptorSize;

      // Remember future physical address for our special relocated
      // efi system table
      BlockSize = EFI_PAGES_TO_SIZE ((UINTN)Desc->NumberOfPages);
      if (Desc->PhysicalStart <= gSysTableRtArea &&  gSysTableRtArea < (Desc->PhysicalStart + BlockSize)) {
        // block contains our future sys table - remember new address
        // future physical = VirtualStart & 0x7FFFFFFFFF
        gRelocatedSysTableRtArea = (Desc->VirtualStart & 0x7FFFFFFFFF) + (gSysTableRtArea - Desc->PhysicalStart);
      }
    }

    Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
  }

  VmFlashCaches ();

  DEBUG ((DEBUG_VERBOSE, "ExecSetVirtualAddressesToMemMap: Size=%d, Addr=%p, DescSize=%d\nSetVirtualAddressMap ... ",
    gVirtualMapSize, MemoryMap, DescriptorSize));
  Status = gRT->SetVirtualAddressMap (gVirtualMapSize, DescriptorSize, DescriptorVersion, gVirtualMemoryMap);
  DEBUG ((DEBUG_VERBOSE, "%r\n", Status));

  return Status;
}

VOID
CopyEfiSysTableToSeparateRtDataArea (
  IN OUT UINT32   *EfiSystemTable
  )
{
  EFI_SYSTEM_TABLE  *Src;
  EFI_SYSTEM_TABLE  *Dest;

  Src = (EFI_SYSTEM_TABLE*)(UINTN)*EfiSystemTable;
  Dest = (EFI_SYSTEM_TABLE*)(UINTN)gSysTableRtArea;

  DEBUG ((DEBUG_VERBOSE, "-Copy %p <- %p, size=0x%lx\n", Dest, Src, Src->Hdr.HeaderSize));
  CopyMem(Dest, Src, Src->Hdr.HeaderSize);

  *EfiSystemTable = (UINT32)(UINTN)Dest;
}

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
      CompareMem(StartOff + FirstOff, SearchSeq, sizeof(SearchSeq)))
      FirstOff++;

    DEBUG ((DEBUG_VERBOSE, "Found first at off %X\n", (UINT32)FirstOff));

    if (StartOff + FirstOff > EndOff) {
      DEBUG ((DEBUG_WARN, "Failed to find first BOOT_MODE_SAFE | BOOT_MODE_ASLR sequence\n"));
      break;
    }

    SecondOff = FirstOff + sizeof(SearchSeq);

    while (
      StartOff + SecondOff <= EndOff && FirstOff + MaxDist >= SecondOff &&
      CompareMem(StartOff + SecondOff, SearchSeq, sizeof(SearchSeq)))
      SecondOff++;

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

VOID
ProcessBooterImage (
  EFI_HANDLE      ImageHandle
  )
{
  EFI_STATUS Status;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

  Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
  if (EFI_ERROR(Status) || LoadedImage->ImageSize < 0x1000) {
    DEBUG ((DEBUG_VERBOSE, "Failed to find sane loaded image protocol %r\n", Status));
    return;
  }

#if APTIOFIX_ALLOW_ASLR_IN_SAFE_MODE == 1
  UnlockSlideSupportForSafeModeAndCheckSlide (
    (UINT8 *)LoadedImage->ImageBase,
    LoadedImage->ImageSize
    );
#endif

  if (LoadedImage->LoadOptions && LoadedImage->LoadOptionsSize > sizeof(CHAR16)) {
    // Just in case we do not have 0-termination
    CHAR16 *Options = (CHAR16 *)LoadedImage->LoadOptions;
    UINTN LastIndex = LoadedImage->LoadOptionsSize/sizeof(CHAR16)-1;
    CHAR16 Last = Options[LastIndex];
    Options[LastIndex] = '\0';

    CHAR16 *Slide = StrStr(Options, L"slide=");
    VERIFY_BOOT_ARG(Slide, Options, L"slide=");
    gSlideArgPresent |= Slide != NULL;

    CHAR16 *Dump  = StrStr(Options, L"-aptiodump");
    VERIFY_BOOT_ARG(Dump,  Options, L"-aptiodump");
    gDumpMemArgPresent |= Dump != NULL;

    Options[LastIndex] = Last;

    if (Slide)
      DEBUG ((DEBUG_VERBOSE, "Found custom slide param\n"));
  }

  CHAR8 BootArgsVar[BOOT_LINE_LENGTH];
  UINTN BootArgsVarLen = BOOT_LINE_LENGTH;

  // Important to avoid triggering boot-args wrapper too early
  Status = ((EFI_GET_VARIABLE)gGetVariable)(
    L"boot-args",
    &gAppleBootVariableGuid,
    NULL, &BootArgsVarLen,
    &BootArgsVar[0]
    );

  if (!EFI_ERROR(Status) && BootArgsVarLen > 0) {
    // Just in case we do not have 0-termination
    BootArgsVar[BootArgsVarLen-1] = '\0';

    CHAR8 *Slide = AsciiStrStr(BootArgsVar, "slide=");
    VERIFY_BOOT_ARG(Slide, BootArgsVar, "slide=");
    gSlideArgPresent |= Slide != NULL;

    CHAR8 *Dump  = AsciiStrStr(BootArgsVar, "-aptiodump");
    VERIFY_BOOT_ARG(Dump,  BootArgsVar, "-aptiodump");
    gDumpMemArgPresent |= Dump != NULL;

    if (Slide)
      DEBUG ((DEBUG_VERBOSE, "Found custom slide boot-arg value\n"));
  }
}

BOOLEAN
IsSandyOrIvy (
  VOID
  )
{
  UINT32  Eax;
  UINT32  CpuFamily;
  UINT32  CpuModel;

  if (!gSandyOrIvySet) {
    Eax = 0;

    AsmCpuid (1, &Eax, NULL, NULL, NULL);

    CpuFamily = (Eax >> 8) & 0xF;
    if (CpuFamily == 15) // Use ExtendedFamily
      CpuFamily = (Eax >> 20) + 15;

    CpuModel = (Eax & 0xFF) >> 4;
    if (CpuFamily == 15 || CpuFamily == 6) // Use ExtendedModel
      CpuModel |= (Eax >> 12) & 0xF0;

    gSandyOrIvy = CpuFamily == 6 && (CpuModel == 0x2A || CpuModel == 0x3A);
    gSandyOrIvySet = TRUE;

    DEBUG ((DEBUG_VERBOSE, "Discovered CpuFamily %d CpuModel %d SandyOrIvy %d\n", CpuFamily, CpuModel, gSandyOrIvy));
  }

  return gSandyOrIvy;
}

VOID
GetSlideRangeForValue (
  UINT8   Slide,
  UINTN   *StartAddr,
  UINTN   *EndAddr
  )
{
  BOOLEAN SandyOrIvy;

  SandyOrIvy = IsSandyOrIvy();

  *StartAddr = (UINTN)Slide * 0x200000 + BASE_KERNEL_ADDR;

  // Skip ranges improperly used by Intel HD 2000/3000.
  if (Slide >= 0x80 && SandyOrIvy)
    *StartAddr += 0x10200000;

  *EndAddr   = *StartAddr + APTIOFIX_SPECULATED_KERNEL_SIZE;
}

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
      Slide != 0)
      break;

    Clock = (UINT32)AsmReadTsc();
    Slide = (Clock & 0xFF) ^ ((Clock >> 8) & 0xFF);
  } while (Slide == 0);

  //Print(L"Generated slide index %d value %d\n", Slide, gValidSlides[Slide % gValidSlidesNum]);

  //FIXME: This is bad due to uneven distribution, but let's use it for now.
  return gValidSlides[Slide % gValidSlidesNum];
}

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
      Print (L"No slide values are available for usage! You should use custom slide.\n");
    } else {
      //
      // Pretty-print valid slides as ranges.
      // For example, 1, 2, 3, 4, 5 will becomes 1-5.
      //
      Print (L"Only %d/%d slide values are available for usage! Booting may fail!\n", gValidSlidesNum, TOTAL_SLIDE_NUM);
      NumEntries = 0;
      for (Index = 0; Index <= gValidSlidesNum; Index++) {
        if (Index == 0) {
          Print (L"Valid slides: %d", gValidSlides[Index]);
        } else if (Index == gValidSlidesNum || gValidSlides[Index - 1] + 1 != gValidSlides[Index]) {
          if (NumEntries == 1)
            Print (L", %d", gValidSlides[Index-1]);
          else if (NumEntries > 1)
            Print (L"-%d", gValidSlides[Index-1]);
          if (Index == gValidSlidesNum)
            Print (L"\n");
          else
            Print (L", %d", gValidSlides[Index]);
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
      // We delay memory map analysis as much as we can, because boot.efi allocates
      // stuff with gBS->AllocatePool and this overlaps with the slide region (e.g. on X299).
      // The solution could be to wrap pool allocation with AllocatePagesFromTop,
      // and keep track of all the allocated pointers to later be able to free them.
      // Some stubs are already present (see AllocPool, FreePool overrides)
      if (!gSlideArgPresent && !gAnalyzeMemoryMapDone) {
        DecideOnCustomSlideImplementation();
        gAnalyzeMemoryMapDone = TRUE;
        //Print(L"Slides were analyzed!\n");
      }

      if (gValidSlidesNum != TOTAL_SLIDE_NUM && gValidSlidesNum > 0) {
        // Only return custom boot-args if gValidSlidesNum were determined to be less than TOTAL_SLIDE_NUM
        // And thus we have to use a custom slide implementation to boot reliably
        IsBootArgs = TRUE;
      }
#endif
    }
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
  BootArgs   *BootArgs
  )
{
  DTEntry     DevTree;
  DTEntry     Chosen;
  CHAR8       *ArgsStr;
  UINTN       ArgsSize;

  // Firstly, there is a BootArgs entry for XNU
  RemoveArgumentFromCommandLine(BootArgs->CommandLine, "slide=");

  // Secondly, there is a DT entry
  DevTree = (DTEntry)(UINTN)(*BootArgs->deviceTreeP);

  DTInit(DevTree);
  if (DTLookupEntry(NULL, "/chosen", &Chosen) == kSuccess) {
    DEBUG ((DEBUG_VERBOSE, "Found /chosen\n"));
    if (DTGetProperty(Chosen, "boot-args", (VOID **)&ArgsStr, &ArgsSize) == kSuccess && ArgsSize > 0) {
      DEBUG ((DEBUG_VERBOSE, "Found boot-args in /chosen\n"));
      RemoveArgumentFromCommandLine(ArgsStr, "slide=");
    }
  }

  // Thirdly clean the boot args just in case
  gValidSlidesNum = 0;
  gStoredBootArgsVarSize = 0;
  SetMem(gValidSlides, sizeof(gValidSlides), 0);
  SetMem(gStoredBootArgsVar, sizeof(gStoredBootArgsVar), 0);
}

VOID
RestoreRelocInfoProtectMemTypes (
  UINTN                   MemoryMapSize,
  UINTN                   DescriptorSize,
  EFI_MEMORY_DESCRIPTOR   *MemoryMap
  )
{
  EFI_MEMORY_DESCRIPTOR   *Desc;
  UINTN Index;
  UINTN Index2;
  UINTN NumEntriesLeft;

  NumEntriesLeft = gRelocInfoData.NumEntries;
  Desc = MemoryMap;

  if (NumEntriesLeft > 0) {
    for (Index = 0; Index < (MemoryMapSize / DescriptorSize); ++Index) {
      if (NumEntriesLeft > 0) {
        for (Index2 = 0; Index2 < gRelocInfoData.NumEntries; ++Index2) {
          if (Desc->PhysicalStart == gRelocInfoData.RelocInfo[Index2].PhysicalStart) {
            Desc->Type = gRelocInfoData.RelocInfo[Index2].Type;
            --NumEntriesLeft;
          }
        }
      }

      Desc = NEXT_MEMORY_DESCRIPTOR (Desc, DescriptorSize);
    }
  }
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
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  EFI_MEMORY_DESCRIPTOR   *Desc;

  RT_RELOC_PROTECT_INFO *RelocInfo;

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;
  DEBUG ((DEBUG_VERBOSE, "FixNvramRelocation\n"));

  gRelocInfoData.NumEntries = 0;

  RelocInfo = &gRelocInfoData.RelocInfo[0];

  for (Index = 0; Index < NumEntries; Index++) {
    if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0 &&
        (Desc->Type == EfiRuntimeServicesCode ||
        (Desc->Type == EfiRuntimeServicesData && Desc->PhysicalStart != gSysTableRtArea))) {

      if (gRelocInfoData.NumEntries < ARRAY_SIZE (gRelocInfoData.RelocInfo)) {
        RelocInfo->PhysicalStart = Desc->PhysicalStart;
        RelocInfo->Type          = Desc->Type;
        ++RelocInfo;
        ++gRelocInfoData.NumEntries;
      } else {
        DEBUG ((DEBUG_WARN, " WARNING: Cannot save mem type for entry: %lx (type 0x%x)\n", Desc->PhysicalStart, (UINTN)Desc->Type));
      }

      DEBUG ((DEBUG_VERBOSE, " RT mem %lx (0x%x) -> MemMapIO\n", Desc->PhysicalStart, Desc->NumberOfPages));
      Desc->Type = EfiMemoryMappedIO;
    }

    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
  }
}

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
  UINTN                   MemoryMapSize,
  EFI_MEMORY_DESCRIPTOR   *MemoryMap,
  UINTN                   DescriptorSize
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

    if (Desc->PhysicalStart < 0xA0000 && PhysicalEnd >= 0x9E000 && Desc->Type == EfiBootServicesData) {
      Desc->Type = EfiACPIMemoryNVS;
      break;
    }

    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
  }
}

/** Fixes stuff when booting without relocation block. Called when boot.efi jumps to kernel. */
UINTN
FixBootingWithoutRelocBlock (
  UINTN    bootArgs,
  BOOLEAN  ModeX64
  )
{
  VOID                    *pBootArgs = (VOID*)bootArgs;
  BootArgs                *BA;
  UINTN                   MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemoryMap;
  UINTN                   DescriptorSize;

  DEBUG ((DEBUG_VERBOSE, "FixBootingWithoutRelocBlock:\n"));

  BA = GetBootArgs(pBootArgs);

  MemoryMapSize = *BA->MemoryMapSize;
  MemoryMap = (EFI_MEMORY_DESCRIPTOR*)(UINTN)(*BA->MemoryMap);
  DescriptorSize = *BA->MemoryMapDescriptorSize;

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

  // We must restore EfiRuntimeServicesCode memory areas, because otherwise
  // RuntimeServices won't be executable.
  RestoreRelocInfoProtectMemTypes(MemoryMapSize, DescriptorSize, MemoryMap);

  // Restore original kernel entry code.
  CopyMem((VOID *)(UINTN)AsmKernelEntry, (VOID *)gOrigKernelCode, gOrigKernelCodeSize);

  return bootArgs;
}

/** Fixes stuff when waking from hibernate without relocation block. Called when boot.efi jumps to kernel. */
UINTN
FixHibernateWakeWithoutRelocBlock (
  UINTN    imageHeaderPage,
  BOOLEAN  ModeX64
  )
{
  IOHibernateImageHeader  *ImageHeader;
  IOHibernateHandoff      *Handoff;

  ImageHeader = (IOHibernateImageHeader *)(UINTN)(imageHeaderPage << EFI_PAGE_SHIFT);

  // Pass our relocated copy of system table
  ImageHeader->systemTableOffset = (UINT32)(UINTN)(gRelocatedSysTableRtArea - ImageHeader->runtimePages);

#if APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP == 1
  // XNU replaces the original restored UEFI mapping by a new one based on kIOHibernateHandoffTypeMemoryMap
  // passed values. This caused instant reboots after hibernation wake for dmazar during the development
  // of the original AptioFixV2 driver. The reasons mentioned were XNU attempts to map the rt pages which
  // overlap with the existing memory.
  //
  // To workaround this issue AptioFixV2 disables memory map handoff, and XNU reuses the original mapping.
  // Due to dynamic allocation memory mapping may sometimes change across the boots, and as a result
  // some of the wakes will fail or result in a memory corruption after some time.
  Handoff = (IOHibernateHandoff *)(UINTN)(ImageHeader->handoffPages << EFI_PAGE_SHIFT);
  while (Handoff->type != kIOHibernateHandoffTypeEnd) {
    if (Handoff->type == kIOHibernateHandoffTypeMemoryMap) {
      Handoff->type = kIOHibernateHandoffType;
      break;
    }
    Handoff = (IOHibernateHandoff *)(UINTN)((UINTN)Handoff + sizeof(Handoff) + Handoff->bytecount);
  }
#else
  // When reusing the original memory mapping we do not have to restore memory protection types & attributes,
  // since the new memory map is discarded anyway.
  // Otherwise we must restore memory map types just like at a normal boot, because MMIO regions are not
  // mapped as executable by XNU.
  //
  // However, there is an issue here. After hibernation restoration we may get corrupted memory, which
  // sometimes results in crashing apps and not working NVRAM. The exact cause is unknown, dumping
  // the memory shows that the handoff memory map is mostly similar, but partially differs.
  //
  // Due to a non-contiguous RT_Code/RT_Data areas (thanks to NVRAM hack) the original areas
  // will not be unmapped and this will result in a memory leak if some new runtime pages are added.
  // But even that should not cause crashes.
  //
  // From the top of my head I could imagine a new memory mapping
  // SystemTable gets a new address, and this address is marked as "Available".
  Handoff = (IOHibernateHandoff *)(UINTN)(ImageHeader->handoffPages << EFI_PAGE_SHIFT);
  while (Handoff->type != kIOHibernateHandoffTypeEnd) {
    if (Handoff->type == kIOHibernateHandoffTypeMemoryMap) {
      // boot.efi removes any memory from the memory map but the one with runtime attribute.
      RestoreRelocInfoProtectMemTypes(Handoff->bytecount, gLastDescriptorSize, (EFI_MEMORY_DESCRIPTOR *)Handoff->data);
      break;
    }
    Handoff = (IOHibernateHandoff *)(UINTN)((UINTN)Handoff + sizeof(Handoff) + Handoff->bytecount);
  }
#endif

  // Restore original kernel entry code
  CopyMem((VOID *)(UINTN)AsmKernelEntry, (VOID *)gOrigKernelCode, gOrigKernelCodeSize);

  return imageHeaderPage;
}
