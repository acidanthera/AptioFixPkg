/**

  Basic functions for parsing Mach-O kernel.

  by dmazar

**/

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

#include "UefiLoader.h"
#include "Mach-O.h"

/** Adds Offset bytes to SourcePtr and returns new pointer as ReturnType. */
#define PTR_OFFSET(SourcePtr, Offset, ReturnType) ((ReturnType)(((UINT8*)SourcePtr) + Offset))

/** Returns Mach-O entry point from LC_UNIXTHREAD loader command. */
UINTN
MachOGetEntryAddress(
  IN VOID  *MachOImage
  )
{
  struct mach_header      *MHdr;
  struct mach_header_64   *MHdr64;
  BOOLEAN                 Is64Bit;
  UINT32                  NCmds;
  struct load_command     *LCmd;
  UINTN                   Index;
  i386_thread_state_t     *ThreadState;
  x86_thread_state64_t    *ThreadState64;
  UINTN                   Address;

  Address = 0;
  MHdr = (struct mach_header *)MachOImage;
  MHdr64 = (struct mach_header_64 *)MachOImage;
  DEBUG ((DEBUG_VERBOSE, "Mach-O image: %p, magic: %x", MachOImage, MHdr->magic));

  if (MHdr->magic == MH_MAGIC || MHdr->magic == MH_CIGAM) {
    //
    // 32-bit header
    //
    DEBUG ((DEBUG_VERBOSE, " -> 32 bit\n"));
    Is64Bit = FALSE;
    NCmds = MHdr->ncmds;
    LCmd = PTR_OFFSET (MachOImage, sizeof(struct mach_header), struct load_command *);
  } else if (MHdr64->magic == MH_MAGIC_64 || MHdr64->magic == MH_CIGAM_64) {
    //
    // 64-bit header
    //
    DEBUG ((DEBUG_VERBOSE, " -> 64 bit\n"));
    Is64Bit = TRUE;
    NCmds = MHdr64->ncmds;
    LCmd = PTR_OFFSET (MachOImage, sizeof(struct mach_header_64), struct load_command *);
  } else {
    //
    // Invalid Mach-O image
    //
    return Address;
  }

  DEBUG ((DEBUG_VERBOSE, "Number of cmds: %d, address: %p\n", NCmds, LCmd));

  //
  // Iterate over load commands
  //
  for (Index = 0; Index < NCmds; Index++) {

    DEBUG ((DEBUG_VERBOSE, "%d. LCmd: %p, cmd: %x, size: %d\n", Index, LCmd, LCmd->cmd, LCmd->cmdsize));

    if (LCmd->cmd == LC_UNIXTHREAD) {

      DEBUG ((DEBUG_VERBOSE, "LC_UNIXTHREAD\n"));
      //
      // extract thread state
      // LCmd =
      //  struct load_command {
      //   uint32_t cmd
      //   uint32_t cmdsize
      //  }
      //  uint32_t flavor        flavor of thread state */
      //  uint32_t count         count of longs in thread state */
      //  struct XXX_thread_state state   thread state for this flavor */
      //
      ThreadState = PTR_OFFSET (LCmd, sizeof(struct load_command) + 2 * sizeof(UINT32), i386_thread_state_t *);
      ThreadState64 = (x86_thread_state64_t *)ThreadState;

      if (Is64Bit) {
        Address = (UINTN)ThreadState64->rip;
      } else {
        Address = (UINTN)ThreadState->eip;
      }
      break;
    }

    LCmd = PTR_OFFSET (LCmd, LCmd->cmdsize, struct load_command *);

  }

  DEBUG ((DEBUG_VERBOSE, "Address: %lx\n", Address));
  return Address;
}
