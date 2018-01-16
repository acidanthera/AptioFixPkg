/**

  Methods for setting callback jump from kernel entry point, callback, fixes to kernel boot image.

  by dmazar

**/

#ifndef APTIOFIX_BOOT_FIXES_H
#define APTIOFIX_BOOT_FIXES_H

typedef struct {
  EFI_PHYSICAL_ADDRESS  PhysicalStart;
  EFI_MEMORY_TYPE       Type;
} RT_RELOC_PROTECT_INFO;

typedef struct {
  UINTN                 NumEntries;
  RT_RELOC_PROTECT_INFO RelocInfo[APTIFIX_MAX_RT_RELOC_NUM];
} RT_RELOC_PROTECT_DATA;

extern EFI_PHYSICAL_ADDRESS   gSysTableRtArea;
extern BOOLEAN                gHibernateWake;
extern BOOLEAN                gSlideArgPresent;
extern BOOLEAN                gDumpMemArgPresent;
extern EFI_GET_MEMORY_MAP     gStoredGetMemoryMap;
extern UINTN                  gLastMemoryMapSize;
extern EFI_MEMORY_DESCRIPTOR  *gLastMemoryMap;
extern UINTN                  gLastDescriptorSize;
extern UINT32                 gLastDescriptorVersion;
extern EFI_PHYSICAL_ADDRESS   gSysTableRtArea;
extern EFI_PHYSICAL_ADDRESS   gRelocatedSysTableRtArea;

EFI_STATUS
PrepareJumpFromKernel (
  VOID
  );

EFI_STATUS
KernelEntryPatchJump (
  UINT32 KernelEntry
  );

EFI_STATUS
KernelEntryFromMachOPatchJump (
  VOID *MachOImage,
  UINTN SlideAddr
  );

EFI_STATUS
ExecSetVirtualAddressesToMemMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap
  );

VOID
CopyEfiSysTableToSeparateRtDataArea (
  IN OUT UINT32             *EfiSystemTable
  );

VOID
ProtectRtMemoryFromRelocation(
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap
  );

VOID
VirtualizeRtShimPointers (
  UINTN                   MemoryMapSize,
  UINTN                   DescriptorSize,
  EFI_MEMORY_DESCRIPTOR   *MemoryMap
  );

VOID
UnlockSlideSupportForSafeModeAndCheckSlide (
  UINT8                   *ImageBase,
  UINTN                   ImageSize
  );

VOID
ProcessBooterImage (
  EFI_HANDLE              ImageHandle
  );

VOID
DecideOnCustomSlideImplementation (
  );

BOOLEAN
IsSandyOrIvy (
  );

EFI_STATUS
EFIAPI
GetVariableCustomSlide (
  CHAR16                  *VariableName,
  EFI_GUID                *VendorGuid,
  UINT32                  *Attributes,
  UINTN                   *DataSize,
  VOID                    *Data
  );

VOID
HideSlideFromOS (
  BootArgs   *BootArgs
  );

/** Fixes stuff for booting without relocation block. Called when boot.efi jumps to kernel. */
UINTN
FixBootingWithoutRelocBlock (
  UINTN   bootArgs,
  BOOLEAN ModeX64
  );

/** Fixes stuff for hibernate wake booting without relocation block. Called when boot.efi jumps to kernel. */
UINTN FixHibernateWakeWithoutRelocBlock (
  UINTN   imageHeaderPage,
  BOOLEAN ModeX64
  );

#endif // APTIOFIX_BOOT_FIXES_H
