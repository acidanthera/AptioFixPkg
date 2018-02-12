/**

  Methods for setting callback jump from kernel entry point, callback, fixes to kernel boot image.

  by dmazar

**/

#ifndef APTIOFIX_BOOT_FIXES_H
#define APTIOFIX_BOOT_FIXES_H

extern EFI_PHYSICAL_ADDRESS   gSysTableRtArea;

extern EFI_PHYSICAL_ADDRESS   gRelocatedSysTableRtArea;

// TRUE if we are doing hibernate wake
extern BOOLEAN gHibernateWake;

// TRUE if booting with -aptiodump
extern BOOLEAN gDumpMemArgPresent;

// TRUE if booting with a manually specified slide=X
extern BOOLEAN gSlideArgPresent;


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
ReadBooterArguments (
  CHAR16 *Options,
  UINTN   OptionsSize
  );

/** Protects CSM regions from the kernel and boot.efi. */
VOID
ProtectCsmRegion (
  UINTN                    MemoryMapSize,
  EFI_MEMORY_DESCRIPTOR    *MemoryMap,
  UINTN                    DescriptorSize
  );

/** Fixes stuff for booting without relocation block. Called when boot.efi jumps to kernel. */
UINTN
FixBooting (
  UINTN   BootArgs
  );

/** Fixes stuff for hibernate wake booting without relocation block. Called when boot.efi jumps to kernel. */
UINTN FixHibernateWake (
  UINTN   ImageHeaderPage
  );

#endif // APTIOFIX_BOOT_FIXES_H
