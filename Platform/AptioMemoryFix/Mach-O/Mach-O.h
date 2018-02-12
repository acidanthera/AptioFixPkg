/**

  Basic functions for parsing Mach-O kernel.

  by dmazar

**/

#ifndef APTIOFIX_MACH_O_H
#define APTIOFIX_MACH_O_H

/** Returns Mach-O entry point from LC_UNIXTHREAD loader command. */
UINTN
MachOGetEntryAddress (
  IN VOID  *MachOImage
  );

#endif //APTIOFIX_MACH_O_H
