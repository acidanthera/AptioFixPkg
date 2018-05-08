/**

  Various reusable utility code.

  by joedm & vit9696

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "Config.h"
#include "Utils.h"

CHAR8 *
ConvertUnicodeStrToAsciiStr (
  IN CONST CHAR16  *Source,
  OUT CHAR8        *Destination,
  IN CONST UINTN   DestinationSize
  )
{
  CHAR8 *ReturnValue;
  if (Destination == NULL || Source == NULL || DestinationSize == 0) {
    return NULL;
  }

  ReturnValue = Destination;
  while (*Source && ((UINTN)(Destination - ReturnValue) < DestinationSize - 1)) {
    *(Destination++) = (CHAR8)(*(Source++) & 0xFF);
  }

  *Destination = '\0';
  return ReturnValue;
}

VOID
EFIAPI
PrintScreen (
  IN  CONST CHAR16   *Format,
  ...
  )
{
  CHAR16 Buffer[1024];
  VA_LIST Marker;

  VA_START (Marker, Format);
  UnicodeVSPrint (Buffer, sizeof (Buffer), Format, Marker);
  VA_END (Marker);

  //
  // It is safe to call gST->ConOut->OutputString, because in crtitical areas we override
  // gBS->AllocatePool with our own implementation that uses a custom allocator.
  //
  if (gST->ConOut)
    gST->ConOut->OutputString (gST->ConOut, Buffer);
}
