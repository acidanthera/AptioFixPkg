/**

  Aptio Memory Fix protocol to inform bootloaders
  about driver availability.

  by cecekpawon, vit9696

**/

#ifndef APTIOFIX_MEMORY_PROTOCOL_H
#define APTIOFIX_MEMORY_PROTOCOL_H

#define APTIOMEMORYFIX_PROTOCOL_REVISION  0x0000000B

// APTIOMEMORYFIX_PROTOCOL_GUID
// C7CBA84E-CC77-461D-9E3C-6BE0CB79A7C1
#define APTIOMEMORYFIX_PROTOCOL_GUID  \
  { 0xC7CBA84E, 0xCC77, 0x461D, { 0x9E, 0x3C, 0x6B, 0xE0, 0xCB, 0x79, 0xA7, 0xC1 } }

// Includes a revision for debugging reasons
typedef struct {
  UINTN   Revision;
} APTIOMEMORYFIX_PROTOCOL;

extern EFI_GUID gAptioMemoryFixProtocolGuid;

#endif // APTIOFIX_MEMORY_PROTOCOL_H
