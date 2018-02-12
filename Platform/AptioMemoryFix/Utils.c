/**

  Various reusable utility code.

  by joedm & vit9696

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "Config.h"
#include "Utils.h"

/** Returns upper case version of char - valid only for ASCII chars in unicode. */
STATIC
CHAR16
ToUpperChar (
  IN CHAR16 Chr
  )
{
  CHAR8   C;

  if ((Chr & 0xFF) != Chr) {
    return Chr;
  }

  C = (CHAR8)Chr;
  return ((C >= 'a' && C <= 'z') ? C - ('a' - 'A') : C);
}

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

CHAR16 *
StrStriBasic (
  IN CONST CHAR16  *String,
  IN CONST CHAR16  *SearchString
  )
{
  CONST CHAR16 *FirstMatch;
  CONST CHAR16 *SearchStringTmp;

  if (*SearchString == L'\0') {
    return (CHAR16 *) String;
  }

  while (*String != L'\0') {
    SearchStringTmp = SearchString;
    FirstMatch = String;

    while ((ToUpperChar(*String) == ToUpperChar(*SearchStringTmp)) 
        && (*String != L'\0')) {
      String++;
      SearchStringTmp++;
    }

    if (*SearchStringTmp == L'\0') {
      return (CHAR16 *) FirstMatch;
    }

    if (*String == L'\0') {
      return NULL;
    }

    String = FirstMatch + 1;
  }

  return NULL;
}

CHAR16 *
FileDevicePathToStr (
  EFI_DEVICE_PATH_PROTOCOL *FilePathProto
  )
{
  FILEPATH_DEVICE_PATH  *FilePath;
  CHAR16                FilePathText[256]; // Possible problem: filepath may be bigger
  CHAR16                *OutFilePathText;
  UINTN                 Size;
  UINTN                 SizeAll;
  UINTN                 Index;

  FilePathText[0] = L'\0';
  Index = 4;
  SizeAll = 0;

  while (FilePathProto != NULL && FilePathProto->Type != END_DEVICE_PATH_TYPE && Index > 0) {
    if (FilePathProto->Type == MEDIA_DEVICE_PATH && FilePathProto->SubType == MEDIA_FILEPATH_DP) {
      FilePath = (FILEPATH_DEVICE_PATH *)FilePathProto;
      Size = (DevicePathNodeLength (FilePathProto) - 4) / sizeof(CHAR16);
      if (SizeAll + Size < ARRAY_SIZE (FilePathText)) {
        if (SizeAll > 0 && FilePathText[SizeAll / sizeof(CHAR16) - sizeof(CHAR16)] != L'\\') {
          StrCatS (FilePathText, ARRAY_SIZE (FilePathText), L"\\");
        }
        StrCatS (FilePathText, ARRAY_SIZE (FilePathText), FilePath->PathName);
        SizeAll = StrSize (FilePathText);
      }
    }
    FilePathProto = NextDevicePathNode (FilePathProto);
    Index--;
  }

  OutFilePathText = NULL;
  Size = StrSize (FilePathText);
  if (Size > sizeof(CHAR16)) {
    OutFilePathText = (CHAR16 *)AllocatePool (Size);
    if (OutFilePathText)
      StrCpyS (OutFilePathText, Size/sizeof(CHAR16), FilePathText);
  }

  return OutFilePathText;
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
  // It is safe to call gST->ConOut->OutputString, because gBS->AllocatePool
  // is overridden by our own implementation with a custom allocator.
  //
  if (gST->ConOut)
    gST->ConOut->OutputString (gST->ConOut, Buffer);
}
