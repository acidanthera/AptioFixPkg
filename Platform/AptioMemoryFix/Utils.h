/**

  Various reusable utility code.

  by joedm & vit9696

**/

#ifndef APTIOFIX_UTILS_H
#define APTIOFIX_UTILS_H

/**
  Returns the actual length of a string literal (ASCII or Unicode)
**/
#define LITERAL_STRLEN(x) (ARRAY_SIZE (x)-1)

/**
  Quick and dirty dec digit printer
**/
#define DEC_TO_ASCII(x) ("0123456789      "[(x) & 0xF])
#define DEC_SPACE 0xF

/**
  Convert seconds to microseconds for use in e.g. gBS->Stall
**/
#define SECONDS_TO_MICROSECONDS(x) ((x)*1000000)

/**
  Convert a Null-terminated Unicode string to a Null-terminated
  ASCII string and returns the ASCII string.
  This function converts the content of the Unicode string Source
  to the ASCII string Destination by copying the lower 8 bits of
  each Unicode character. It returns Destination.
  The caller is responsible to make sure Destination points to a buffer with size
  equal or greater than ((StrLen (Source) + 1) * sizeof (CHAR8)) in bytes.
  The caller is responsible that Source and Destination don't overlap.
  If Destination is NULL, or if Source is NULL, then NULL is returned.
  If DestinationSize is zero, it's a no-op and Destination returned.
  @param  Source           A pointer to a Null-terminated Unicode string.
  @param  Destination      A pointer to a Null-terminated ASCII string.
  @param  DestinationSize  A size of Destination string, including null-terminating char.
  @return Destination.
**/
CHAR8 *
ConvertUnicodeStrToAsciiStr (
  IN     CONST CHAR16  *Source,
     OUT CHAR8         *Destination,
  IN     CONST UINTN   DestinationSize
  );

/** 
  Returns the first occurrence of a Null-terminated Unicode SearchString
  in a Null-terminated Unicode String.
  Compares just first 8 bits of chars (valid for ASCII), case insensitive.
  Copied from MdePkg/Library/BaseLib/String.c and modified.
  @param  String          A pointer to a Null-terminated Unicode string.
  @param  SearchString    A pointer to a Null-terminated Unicode string to search for.
  @retval NULL            If the SearchString does not appear in String.
  @return others          If there is a match.
**/
CHAR16 *
StrStriBasic (
  IN CONST CHAR16  *String,
  IN CONST CHAR16  *SearchString
  );

/**
  Returns file path from FilePath device path in pool allocated memory.
  Memory should be released by the caller.
**/
CHAR16 *
FileDevicePathToStr (
  EFI_DEVICE_PATH_PROTOCOL  *FilePathProto
  );

/**
  Prints via gST->ConOut without any pool allocations.
  Otherwise equivalent to Print.
  Note: EFIAPI must be present for VA_ARGS forwarding (causes bugs with gcc).
**/
VOID
EFIAPI
PrintScreen (
  IN CONST CHAR16  *Format,
  ...
  );

#endif // APTIOFIX_UTILS_H
