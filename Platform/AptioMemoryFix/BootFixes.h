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

extern EFI_PHYSICAL_ADDRESS   gRelocatedSysTableRtArea;

// TRUE if we are doing hibernate wake
extern BOOLEAN gHibernateWake;

// TRUE if booting with -aptiodump
extern BOOLEAN gDumpMemArgPresent;

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
ProcessBooterImage (
  EFI_HANDLE              ImageHandle
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
FixBooting(
  UINTN   BootArgs
  );

/** Fixes stuff for hibernate wake booting without relocation block. Called when boot.efi jumps to kernel. */
UINTN FixHibernateWake (
  UINTN   ImageHeaderPage
  );

#endif // APTIOFIX_BOOT_FIXES_H
