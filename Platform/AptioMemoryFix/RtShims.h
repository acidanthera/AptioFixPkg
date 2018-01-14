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
extern UINTN gGetVariableBoot;

extern UINTN RtShimGetVariable;
extern UINTN RtShimGetNextVariableName;
extern UINTN RtShimSetVariable;

extern VOID *gRtShims;

#endif // APTIOFIX_RT_SHIMS_H
