/**

  MemoryMap helper functions.

  by dmazar

**/

#ifndef APTIOFIX_MEMORY_MAP_H
#define APTIOFIX_MEMORY_MAP_H

/** Protects AMI CSM region from being overwritten by the kernel. */
VOID
ProtectCsmRegion (
  UINTN                  MemoryMapSize,
  EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  UINTN                  DescriptorSize
  );

/** Prints mem map. */
VOID
PrintMemMap (
  IN CONST CHAR16           *Name,
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN EFI_PHYSICAL_ADDRESS   SysTable
  );

#endif // APTIOFIX_MEMORY_MAP_H
