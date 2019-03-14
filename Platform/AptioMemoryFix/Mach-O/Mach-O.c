/**

  Basic functions for parsing Mach-O kernel.

  by dmazar

**/

#include <IndustryStandard/AppleMachoImage.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

#include "Mach-O.h"

/** Returns Mach-O entry point from LC_UNIXTHREAD loader command. */
UINTN
MachOGetEntryAddress (
  IN VOID  *MachOImage
  )
{
  MACH_HEADER_ANY         *MHdr;
  BOOLEAN                 Is64Bit;
  UINT32                  NCmds;
  MACH_LOAD_COMMAND       *LCmd;
  UINTN                   Index;
  MACH_THREAD_COMMAND     *ThreadCmd;
  MACH_X86_THREAD_STATE   *ThreadState;
  UINTN                   Address;

  Address = 0;
  MHdr    = (MACH_HEADER_ANY *) MachOImage;

  if (MHdr->Signature == MACH_HEADER_SIGNATURE) {
    //
    // 32-bit header.
    //
    Is64Bit = FALSE;
    NCmds   = MHdr->Header32.NumCommands;
    LCmd    = &MHdr->Header32.Commands[0];
  } else if (MHdr->Signature == MACH_HEADER_64_SIGNATURE) {
    //
    // 64-bit header.
    //
    Is64Bit = TRUE;
    NCmds   = MHdr->Header64.NumCommands;
    LCmd    = &MHdr->Header64.Commands[0];
  } else {
    //
    // Invalid Mach-O image.
    //
    return Address;
  }

  //
  // Iterate over load commands.
  //
  for (Index = 0; Index < NCmds; ++Index) {
    if (LCmd->CommandType == MACH_LOAD_COMMAND_UNIX_THREAD) {
      ThreadCmd     = (MACH_THREAD_COMMAND *) LCmd;
      ThreadState   = (MACH_X86_THREAD_STATE *) &ThreadCmd->ThreadState[0];
      Address       = Is64Bit ? ThreadState->State64.rip : ThreadState->State32.eip;
      break;
    }

    LCmd = NEXT_MACH_LOAD_COMMAND (LCmd);
  }

  return Address;
}
