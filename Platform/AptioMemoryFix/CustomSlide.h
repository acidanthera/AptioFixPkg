/**

    Header file for CustomSlide.c

 */
#ifndef APTIOFIX_CUSTOM_SLIDE_H
#define APTIOFIX_CUSTOM_SLIDE_H

// Forward declarations
struct BootArguments;

/**
 * Applies custom 'slide' value fix.
 * @param LoadedImage
 * @return VOID
 */
VOID
ProcessBooterImageForCustomSlide (
    VOID
);

/**
 * Fixes boot-args, ex., removes 'slide' arg.
 * @return VOID
 */
VOID
FixBootingForCustomSlide(
    struct BootArguments *BA
);

/**
 * TODO: provide a comment.
 * @param ImageBase
 * @param ImageSize
 * @return VOID
 */
VOID
UnlockSlideSupportForSafeModeAndCheckSlide (
    UINT8 *ImageBase,
    UINTN ImageSize
);

/**
 * TODO: provide a comment.
 * @return EFI_STATUS
 */
EFI_STATUS
EFIAPI
GetVariableCustomSlide (
    CHAR16                  *VariableName,
    EFI_GUID                *VendorGuid,
    UINT32                  *Attributes,
    UINTN                   *DataSize,
    VOID                    *Data
);

/**
 * Removes 'slide' argument from boot args in order 'slide' is not visible after OS started.
 * @param BootArgs - parsed
 * @return VOID
 */
VOID
HideSlideFromOS (
    struct BootArguments   *BootArgs
);

/**
 * Checks whether the area overlaps with a possible kernel image area.
 * Returns TRUE if the given mem area overlaps, otherwise returns FALSE.
 * @return TRUE or FALSE
 */
BOOLEAN
OverlapsWithSlide (
    EFI_PHYSICAL_ADDRESS   Address,
    UINTN                  Size
);

#endif // APTIOFIX_CUSTOM_SLIDE_H