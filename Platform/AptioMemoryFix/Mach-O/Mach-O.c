/**

  Basic functions for parsing Mach-O kernel.

  by dmazar

**/

#include <IndustryStandard/AppleMachoImage.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

#include "Mach-O.h"

/** Adds Offset bytes to SourcePtr and returns new pointer as ReturnType. */
#define PTR_OFFSET(SourcePtr, Offset, ReturnType) ((ReturnType)(((UINT8*)SourcePtr) + Offset))

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
  MHdr = (MACH_HEADER_ANY *) MachOImage;
  DEBUG ((DEBUG_VERBOSE, "Mach-O image: %p, magic: %x", MachOImage, MHdr->Signature));

  if (MHdr->Signature == MACH_HEADER_SIGNATURE || MHdr->Signature == MACH_HEADER_INVERT_SIGNATURE) {
    //
    // 32-bit header.
    //
    DEBUG ((DEBUG_VERBOSE, " -> 32 bit\n"));
    Is64Bit = FALSE;
    NCmds   = MHdr->Header32.NumCommands;
    LCmd    = &MHdr->Header32.Commands[0];
  } else if (MHdr->Signature == MACH_HEADER_64_SIGNATURE || MHdr->Signature == MACH_HEADER_64_INVERT_SIGNATURE) {
    //
    // 64-bit header.
    //
    DEBUG ((DEBUG_VERBOSE, " -> 64 bit\n"));
    Is64Bit = TRUE;
    NCmds   = MHdr->Header64.NumCommands;
    LCmd    = &MHdr->Header64.Commands[0];
  } else {
    //
    // Invalid Mach-O image.
    //
    return Address;
  }

  DEBUG ((DEBUG_VERBOSE, "Number of cmds: %d, address: %p\n", NCmds, LCmd));

  //
  // Iterate over load commands.
  //
  for (Index = 0; Index < NCmds; Index++) {
    DEBUG ((DEBUG_VERBOSE, "%d. LCmd: %p, cmd: %x, size: %d\n", Index, LCmd, LCmd->CommandType, LCmd->CommandSize));

    if (LCmd->CommandType == MACH_LOAD_COMMAND_UNIX_THREAD) {
      DEBUG ((DEBUG_VERBOSE, "LC_UNIXTHREAD\n"));
      
      //
      // Extract thread state.
      //
      ThreadCmd     = (MACH_THREAD_COMMAND *) LCmd;
      ThreadState   = (MACH_X86_THREAD_STATE *) &ThreadCmd->ThreadState[0];

      if (Is64Bit) {
        Address = ThreadState->State64->rip;
      } else {
        Address = ThreadState->State32->eip;
      }

      break;
    }

    LCmd = NEXT_MACH_LOAD_COMMAND (LCmd);
  }

  DEBUG ((DEBUG_VERBOSE, "Address: %lx\n", Address));
  return Address;
}
