/**

  Various reusable utility code.

  by joedm & vit9696

**/

#include <Library/DebugLib.h>
#include "AsmFuncs.h"
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
