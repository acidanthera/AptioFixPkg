/**
  Various reusable utility code.
*/

#include "Utils.h"

// Detect Sandy and Ivy Bridge CPU
STATIC BOOLEAN gSandyOrIvy = FALSE;
STATIC BOOLEAN gSandyOrIvySet = FALSE;

BOOLEAN
IsSandyOrIvy (
    VOID
)
{
  UINT32  Eax;
  UINT32  CpuFamily;
  UINT32  CpuModel;

  if (!gSandyOrIvySet) {
    Eax = 0;

    AsmCpuid (1, &Eax, NULL, NULL, NULL);

    CpuFamily = (Eax >> 8) & 0xF;
    if (CpuFamily == 15) { // Use ExtendedFamily
      CpuFamily = (Eax >> 20) + 15;
    }

    CpuModel = (Eax & 0xFF) >> 4;
    if (CpuFamily == 15 || CpuFamily == 6) { // Use ExtendedModel
      CpuModel |= (Eax >> 12) & 0xF0;
    }

    gSandyOrIvy = CpuFamily == 6 && (CpuModel == 0x2A || CpuModel == 0x3A);
    gSandyOrIvySet = TRUE;

    DEBUG ((DEBUG_VERBOSE, "Discovered CpuFamily %d CpuModel %d SandyOrIvy %d\n", CpuFamily, CpuModel, gSandyOrIvy));
  }

  return gSandyOrIvy;
}

