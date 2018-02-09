/**
  Header file for Utils.c
 */
#ifndef APTIOFIX_UTILS_H
#define APTIOFIX_UTILS_H

#include <Library/DebugLib.h>
#include "AsmFuncs.h"

/**
 * Detects CPU family and returns TRUE if CPU family is SandyBridge or IvyBridge. Otherwise returns FALSE.
 * @return TRUE or FALSE
 */
BOOLEAN
IsSandyOrIvy (
    VOID
);

/**
 * Just prints a one-byte string byte by byte using DEBUG macro.
 * @return VOID
 */
VOID
PrintSample2(
    UINT8 *sample,
    UINTN size
);

#endif // APTIOFIX_UTILS_H