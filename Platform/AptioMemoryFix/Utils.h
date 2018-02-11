/**
  Header file for Utils.c
 */
#ifndef APTIOFIX_UTILS_H
#define APTIOFIX_UTILS_H

/**
 * Detects CPU family and returns TRUE if CPU family is SandyBridge or IvyBridge. Otherwise returns FALSE.
 * @return TRUE or FALSE
 */
BOOLEAN
IsSandyOrIvy (
    VOID
);

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
  @param  Source        A pointer to a Null-terminated Unicode string.
  @param  Destination   A pointer to a Null-terminated ASCII string.
  @return Destination.
**/
CHAR8 *
ConvertUnicodeStrToAsciiStr (
    IN CONST CHAR16  *Source,
    OUT CHAR8        *Destination
);

#endif // APTIOFIX_UTILS_H