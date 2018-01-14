/**

  Various helper functions.

  by dmazar

**/

#ifndef APTIOFIX_LIB_H
#define APTIOFIX_LIB_H

/** For type to string conversion */
extern CHAR16 *mEfiMemoryTypeDesc[EfiMaxMemoryType];
extern CHAR16 *mEfiAllocateTypeDesc[MaxAllocateType];
extern CHAR16 *mEfiLocateSearchType[];

extern EFI_ALLOCATE_POOL  gStoredAllocatePool;
extern EFI_FREE_POOL      gStoredFreePool;

/** MemMap reversed scan */
#define PREV_MEMORY_DESCRIPTOR(MemoryDescriptor, Size) \
	((EFI_MEMORY_DESCRIPTOR *)((UINT8 *)(MemoryDescriptor) - (Size)))

/** Quick and dirty dec digit printer */
#define DEC_TO_ASCII(Val) ("0123456789      "[(Val) & 0xF])
#define DEC_SPACE 0xF

/** Map of known guids and friendly names. Searchable with GuidStr() */
typedef struct {
	EFI_GUID    *Guid;
	CHAR16      *Str;
} MAP_EFI_GUID_STR;

/** Known guids provided by different dependencies (compatibility hack) */
extern EFI_GUID gEfiConsoleControlProtocolGuid;
extern EFI_GUID gAppleFirmwarePasswordProtocolGuid;
extern EFI_GUID gEfiDevicePathPropertyDatabaseProtocolGuid;
extern EFI_GUID gAppleBootVariableGuid;
extern EFI_GUID gAppleVendorVariableGuid;
extern EFI_GUID gAppleFramebufferInfoProtocolGuid;
extern EFI_GUID gAppleKeyMapAggregatorProtocolGuid;
extern EFI_GUID gAppleNetBootProtocolGuid;
extern EFI_GUID gAppleImageCodecProtocolGuid;
extern EFI_GUID gAppleTrbSecureVariableGuid;
extern EFI_GUID gEfiDataHubProtocolGuid;
extern EFI_GUID gEfiMiscSubClassGuid;
extern EFI_GUID gEfiProcessorSubClassGuid;
extern EFI_GUID gEfiMemorySubClassGuid;
extern EFI_GUID gEfiLegacy8259ProtocolGuid;

/** Returns GUID as string, with friendly name for known guids. */
CHAR16*
EFIAPI
GuidStr (
	IN EFI_GUID *Guid
	);

/** Returns upper case version of char - valid only for ASCII chars in unicode. */
CHAR16
EFIAPI
ToUpperChar (
	IN CHAR16 Chr
	);

/** Returns the first occurrence of a SearchString in a String.
  * Compares just first 8 bits of chars (valid for ASCII), case insensitive.
  */
CHAR16*
EFIAPI
StrStriBasic (
	IN CONST CHAR16 *String,
	IN CONST CHAR16 *SearchString
	);

/** Applies some fixes to mem map. */
VOID
EFIAPI
FixMemMap (
	IN UINTN                    MemoryMapSize,
	IN EFI_MEMORY_DESCRIPTOR    *MemoryMap,
	IN UINTN                    DescriptorSize,
	IN UINT32                   DescriptorVersion
	);

/** Shrinks mem map by joining EfiBootServicesCode and EfiBootServicesData records. */
VOID
EFIAPI
ShrinkMemMap (
	IN UINTN                    *MemoryMapSize,
	IN EFI_MEMORY_DESCRIPTOR    *MemoryMap,
	IN UINTN                    DescriptorSize,
	IN UINT32                   DescriptorVersion
	);

/** Prints mem map. */
VOID
EFIAPI
PrintMemMap (
	IN UINTN                    MemoryMapSize,
	IN UINTN                    DescriptorSize,
	IN EFI_MEMORY_DESCRIPTOR    *MemoryMap
	);

/** Prints some values from Sys table and Runt. services. */
VOID
EFIAPI
PrintSystemTable (
	IN EFI_SYSTEM_TABLE  *ST
	);

/** Prints Message and waits for a key press. */
VOID
WaitForKeyPress (
	CHAR16 *Message
	);

/** Returns file path from FilePath device path in allocated memory. Mem should be released by caller.*/
CHAR16 *
EFIAPI
FileDevicePathToText (
	EFI_DEVICE_PATH_PROTOCOL *FilePathProto
	);

/** Helper function that calls GetMemoryMap(), allocates space for mem map and returns it. */
EFI_STATUS
EFIAPI
GetMemoryMapAlloc (
	IN EFI_GET_MEMORY_MAP       GetMemoryMapFunction,
	OUT UINTN                   *MemoryMapSize,
	OUT EFI_MEMORY_DESCRIPTOR   **MemoryMap,
	OUT UINTN                   *MapKey,
	OUT UINTN                   *DescriptorSize,
	OUT UINT32                  *DescriptorVersion
	);

/** Alloctes Pages from the top of mem, up to address specified in Memory. Returns allocated address in Memory. */
EFI_STATUS
EFIAPI
AllocatePagesFromTop (
	IN EFI_MEMORY_TYPE          MemoryType,
	IN UINTN                    Pages,
	IN OUT EFI_PHYSICAL_ADDRESS *Memory
	);

/** Calls real gBS->AllocatePool and returns pool memory. */
VOID *
DirectAllocatePool (
	UINTN     Size
	);

/** Calls real gBS->FreePool and frees pool memory. */
VOID
DirectFreePool (
	VOID      *Buffer
	);

#endif // APTIOFIX_LIB_H
