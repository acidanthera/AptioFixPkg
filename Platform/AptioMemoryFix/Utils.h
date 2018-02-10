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

#endif // APTIOFIX_UTILS_H