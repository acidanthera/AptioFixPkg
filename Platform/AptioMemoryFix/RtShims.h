/**

  Runtime Services Wrappers.

  by Download-Fritz & vit9696

**/

#ifndef APTIOFIX_RT_SHIMS_H
#define APTIOFIX_RT_SHIMS_H

extern UINTN gRtShimsDataStart;
extern UINTN gRtShimsDataEnd;

extern UINTN gGetVariable;
extern UINTN gGetNextVariableName;
extern UINTN gSetVariable;
extern UINTN gGetTime;
extern UINTN gSetTime;
extern UINTN gGetWakeupTime;
extern UINTN gSetWakeupTime;
extern UINTN gGetNextHighMonoCount;
extern UINTN gResetSystem;
extern UINTN gGetVariableBoot;

extern UINTN RtShimGetVariable;
extern UINTN RtShimGetNextVariableName;
extern UINTN RtShimSetVariable;
extern UINTN RtShimGetTime;
extern UINTN RtShimSetTime;
extern UINTN RtShimGetWakeupTime;
extern UINTN RtShimSetWakeupTime;
extern UINTN RtShimGetNextHighMonoCount;
extern UINTN RtShimResetSystem;

extern VOID *gRtShims;

#endif // APTIOFIX_RT_SHIMS_H
