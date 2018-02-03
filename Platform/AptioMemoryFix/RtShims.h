/**

  Runtime Services Wrappers.

  by Download-Fritz & vit9696

**/

#ifndef APTIOFIX_RT_SHIMS_H
#define APTIOFIX_RT_SHIMS_H

extern UINTN gGetVariable;
extern UINTN gGetNextVariableName;
extern UINTN gSetVariable;
extern UINTN gGetTime;
extern UINTN gSetTime;
extern UINTN gGetWakeupTime;
extern UINTN gSetWakeupTime;
extern UINTN gGetNextHighMonoCount;
extern UINTN gResetSystem;
extern UINTN gGetVariableOverride;

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
  UINTN                   MemoryMapSize,
  UINTN                   DescriptorSize,
  EFI_MEMORY_DESCRIPTOR   *MemoryMap
  );

#endif // APTIOFIX_RT_SHIMS_H
