/**

  Runtime Services Wrappers.

  by Download-Fritz & vit9696

**/

#ifndef APTIOFIX_RT_SHIMS_H
#define APTIOFIX_RT_SHIMS_H

extern VOID *gRtShims;

typedef struct {
  UINTN           *gFunc;
  UINTN           *Func;
  BOOLEAN         Fixed;
} ShimPtrs;

VOID
InstallRtShims (
  EFI_GET_VARIABLE GetVariableOverride
  );

VOID
UninstallRtShims (
  VOID
  );

VOID
VirtualizeRtShims (
  UINTN                  MemoryMapSize,
  UINTN                  DescriptorSize,
  EFI_MEMORY_DESCRIPTOR  *MemoryMap
  );

EFI_STATUS
EFIAPI
OrgGetVariable (
  IN     CHAR16    *VariableName,
  IN     EFI_GUID  *VendorGuid,
  OUT    UINT32    *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT    VOID      *Data
  );

#endif // APTIOFIX_RT_SHIMS_H
