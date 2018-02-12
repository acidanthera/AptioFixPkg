/**

  Temporary BS and RT overrides for boot.efi support.
  Unlike RtShims they do not affect the kernel.

  by dmazar

**/

#ifndef APTIOFIX_SERVICE_OVERRIDES_H
#define APTIOFIX_SERVICE_OVERRIDES_H

//
// Minimum and maximum addresses allocated by AlocatePages
//
extern EFI_PHYSICAL_ADDRESS        gMinAllocatedAddr;
extern EFI_PHYSICAL_ADDRESS        gMaxAllocatedAddr;

//
// Last descriptor size obtained from GetMemoryMap
//
extern UINTN                       gMemoryMapDescriptorSize;

VOID
InstallBsOverrides (
  VOID
  );

VOID
UninstallBsOverrides (
  VOID
  );

VOID
InstallRtOverrides (
  VOID
  );

VOID
UninstallRtOverrides (
  VOID
  );

EFI_STATUS
EFIAPI
MOHandleProtocol (
  IN     EFI_HANDLE  Handle,
  IN     EFI_GUID    *Protocol,
     OUT VOID        **Interface
  );

EFI_STATUS
EFIAPI
MOAllocatePages (
  IN     EFI_ALLOCATE_TYPE     Type,
  IN     EFI_MEMORY_TYPE       MemoryType,
  IN     UINTN                 NumberOfPages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory
  );

EFI_STATUS
EFIAPI
MOAllocatePool (
  IN     EFI_MEMORY_TYPE  Type,
  IN     UINTN            Size,
     OUT VOID             **Buffer
  );

EFI_STATUS
EFIAPI
MOFreePool (
  IN VOID  *Buffer
  );

EFI_STATUS
EFIAPI
MOGetMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion
  );

EFI_STATUS
EFIAPI
OrgGetMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion
  );

EFI_STATUS
EFIAPI
MOExitBootServices (
  IN EFI_HANDLE  ImageHandle,
  IN UINTN       MapKey
  );

EFI_STATUS
ForceExitBootServices (
  IN EFI_EXIT_BOOT_SERVICES  ExitBs,
  IN EFI_HANDLE              ImageHandle,
  IN UINTN                   MapKey
  );

EFI_STATUS
EFIAPI
MOSetVirtualAddressMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *VirtualMap
  );

#endif // APTIOFIX_SERVICE_OVERRIDES_H
