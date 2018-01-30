/**

  Various helper functions.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DiskIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/SimpleFileSystem.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>

#include "Config.h"
#include "Lib.h"

CHAR16 *mEfiMemoryTypeDesc[EfiMaxMemoryType] = {
  L"Reserved",
  L"LDR_code",
  L"LDR_data",
  L"BS_code",
  L"BS_data",
  L"RT_code",
  L"RT_data",
  L"Available",
  L"Unusable",
  L"ACPI_recl",
  L"ACPI_NVS",
  L"MemMapIO",
  L"MemPortIO",
  L"PAL_code"
};

CHAR16 *mEfiAllocateTypeDesc[MaxAllocateType] = {
  L"AllocateAnyPages",
  L"AllocateMaxAddress",
  L"AllocateAddress"
};

CHAR16 *mEfiLocateSearchType[] = {
  L"AllHandles",
  L"ByRegisterNotify",
  L"ByProtocol"
};

// From xnu: pexpert/i386/efi.h (legacy no longer used variable guid?)
EFI_GUID mAppleVendorGuid                            = {0xAC39C713, 0x7E50, 0x423D, {0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6}};
// Unknown (no longer used?) guids generally from Clover and rEFInd
EFI_GUID mDataHubOptionsGuid                         = {0x0021001C, 0x3CE3, 0x41F8, {0x99, 0xC6, 0xEC, 0xF5, 0xDA, 0x75, 0x47, 0x31}};
EFI_GUID mNotifyMouseActivity                        = {0xF913C2C2, 0x5351, 0x4FDB, {0x93, 0x44, 0x70, 0xFF, 0xED, 0xB8, 0x42, 0x25}};
EFI_GUID mMsgLogProtocolGuid                         = {0x511CE018, 0x0018, 0x4002, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x03}};

MAP_EFI_GUID_STR EfiGuidStrMap[] = {
#if !defined (MDEPKG_NDEBUG)
  {&gEfiFileInfoGuid, L"gEfiFileInfoGuid"},
  {&gEfiFileSystemInfoGuid, L"gEfiFileSystemInfoGuid"},
  {&gEfiFileSystemVolumeLabelInfoIdGuid, L"gEfiFileSystemVolumeLabelInfoIdGuid"},
  {&gEfiLoadedImageProtocolGuid, L"gEfiLoadedImageProtocolGuid"},
  {&gEfiDevicePathProtocolGuid, L"gEfiDevicePathProtocolGuid"},
  {&gEfiSimpleFileSystemProtocolGuid, L"gEfiSimpleFileSystemProtocolGuid"},
  {&gEfiBlockIoProtocolGuid, L"gEfiBlockIoProtocolGuid"},
  {&gEfiBlockIo2ProtocolGuid, L"gEfiBlockIo2ProtocolGuid"},
  {&gEfiDiskIoProtocolGuid, L"gEfiDiskIoProtocolGuid"},
  {&gEfiDiskIo2ProtocolGuid, L"gEfiDiskIo2ProtocolGuid"},
  {&gEfiGraphicsOutputProtocolGuid, L"gEfiGraphicsOutputProtocolGuid"},

  {&gEfiConsoleControlProtocolGuid, L"gEfiConsoleControlProtocolGuid"},
  {&gAppleFirmwarePasswordProtocolGuid, L"gAppleFirmwarePasswordProtocolGuid"},
  {&gEfiGlobalVariableGuid, L"gEfiGlobalVariableGuid"},
  {&gEfiDevicePathPropertyDatabaseProtocolGuid, L"gEfiDevicePathPropertyDatabaseProtocolGuid"},
  {&gAppleBootVariableGuid, L"gAppleBootVariableGuid"},
  {&gAppleVendorVariableGuid, L"gAppleVendorVariableGuid"},
  {&gAppleFramebufferInfoProtocolGuid, L"gAppleFramebufferInfoProtocolGuid"},
  {&gAppleKeyMapAggregatorProtocolGuid, L"gAppleKeyMapAggregatorProtocolGuid"},
  {&gAppleNetBootProtocolGuid, L"gAppleNetBootProtocolGuid"},
  {&gAppleImageCodecProtocolGuid, L"gAppleImageCodecProtocolGuid"},
  {&mAppleVendorGuid, L"mAppleVendorGuid"},
  {&gAppleTrbSecureVariableGuid, L"gAppleTrbSecureVariableGuid"},
  {&mDataHubOptionsGuid, L"mDataHubOptionsGuid"},
  {&mNotifyMouseActivity, L"mNotifyMouseActivity"},
  {&gEfiDataHubProtocolGuid, L"gEfiDataHubProtocolGuid"},
  {&gEfiMiscSubClassGuid, L"gEfiMiscSubClassGuid"},
  {&gEfiProcessorSubClassGuid, L"gEfiProcessorSubClassGuid"},
  {&gEfiMemorySubClassGuid, L"gEfiMemorySubClassGuid"},
  {&mMsgLogProtocolGuid, L"mMsgLogProtocolGuid"},
  {&gEfiLegacy8259ProtocolGuid, L"gEfiLegacy8259ProtocolGuid"},
#endif
  {NULL, NULL}
};

CHAR16 EfiGuidStrTmp[48];

/** Returns GUID as string, with friendly name for known guids. */
CHAR16 *
EFIAPI
GuidStr (
  IN EFI_GUID *Guid
  )
{
  UINTN       i;
  CHAR16      *Str = NULL;

  for(i = 0; EfiGuidStrMap[i].Guid != NULL; i++) {
    if (CompareGuid(EfiGuidStrMap[i].Guid, Guid)) {
      Str = EfiGuidStrMap[i].Str;
      break;
    }
  }

  if (Str == NULL) {
    UnicodeSPrint(EfiGuidStrTmp, sizeof(EfiGuidStrTmp) - sizeof(EfiGuidStrTmp[0]), L"%g", Guid);
    Str = EfiGuidStrTmp;
  }
  return Str;
}

/** Returns upper case version of char - valid only for ASCII chars in unicode. */
CHAR16
EFIAPI
ToUpperChar (
  IN CHAR16 Chr
  )
{
  CHAR8   C;

  if (Chr > 0xFF) return Chr;
  C = (CHAR8)Chr;
  return ((C >= 'a' && C <= 'z') ? C - ('a' - 'A') : C);
}

/** Returns the first occurrence of a Null-terminated Unicode SearchString
  * in a Null-terminated Unicode String.
  * Compares just first 8 bits of chars (valid for ASCII), case insensitive.
  * Copied from MdePkg/Library/BaseLib/String.c and modified
  */
CHAR16 *
EFIAPI
StrStriBasic (
  IN CONST CHAR16   *String,
  IN CONST CHAR16   *SearchString
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

/** Returns TRUE if String1 starts with String2, FALSE otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive.. */
BOOLEAN
EFIAPI
StriStartsWithBasic(
  IN CHAR16 *String1,
  IN CHAR16 *String2
  )
{
  CHAR16   Chr1;
  CHAR16   Chr2;
  BOOLEAN  Result;

  if (String1 == NULL || String2 == NULL) {
    return FALSE;
  }
  if (*String1 == L'\0' && *String2 == L'\0') {
    return TRUE;
  }
  if (*String1 == L'\0' || *String2 == L'\0') {
    return FALSE;
  }

  Chr1 = ToUpperChar(*String1);
  Chr2 = ToUpperChar(*String2);
  while ((Chr1 != L'\0') && (Chr2 != L'\0') && (Chr1 == Chr2)) {
    String1++;
    String2++;
    Chr1 = ToUpperChar(*String1);
    Chr2 = ToUpperChar(*String2);
  }

  Result = ((Chr1 == L'\0') && (Chr2 == L'\0'))
  || ((Chr1 != L'\0') && (Chr2 == L'\0'));

  return Result;
}

VOID
EFIAPI
FixMemMap (
  IN UINTN                   MemoryMapSize,
  IN EFI_MEMORY_DESCRIPTOR   *MemoryMap,
  IN UINTN                   DescriptorSize,
  IN UINT32                  DescriptorVersion
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  UINTN                   BlockSize;
  UINTN                   PhysicalEnd;

  DEBUG ((DEBUG_VERBOSE, "FixMemMap: Size=%d, Addr=%p, DescSize=%d\n", MemoryMapSize, MemoryMap, DescriptorSize));

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;

  for (Index = 0; Index < NumEntries; Index++) {
    BlockSize = EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages);
    PhysicalEnd = Desc->PhysicalStart + BlockSize;

#if APTIOFIX_PROTECT_IGPU_RESERVED_MEMORY == 1
    //
    // Some UEFIs end up with "reserved" area with EFI_MEMORY_RUNTIME flag set when Intel HD3000 or HD4000 is used.
    // boot.efi does not assign a virtual address in this case, which may result in improper mappings.
    //
    if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0 && Desc->Type == EfiReservedMemoryType) {
      DEBUG ((DEBUG_VERBOSE, " %s as RT: %lx (0x%x) - Att: %lx",
        mEfiMemoryTypeDesc[Desc->Type], Desc->PhysicalStart, Desc->NumberOfPages, Desc->Attribute));
      Desc->Type = EfiMemoryMappedIO;
      DEBUG ((DEBUG_VERBOSE, " -> %lx\n", Desc->Attribute));
    }
#endif

#if APTIOFIX_UNMARKED_OVERLAPPING_REGION_FIX == 1
    //
    // Fix by Slice - fixes sleep/wake on GB boards.
    // Appears that some motherboards have a conventional memory region, when it is actually
    // used in runtime and causes sleep issues & memory corruption.
    //
    if (Desc->PhysicalStart < 0xa0000 && PhysicalEnd >= 0x9e000 && 
      (Desc->Type == EfiConventionalMemory || Desc->Type == EfiBootServicesData || Desc->Type == EfiBootServicesCode)) {
      Desc->Type = EfiACPIMemoryNVS;
      Desc->Attribute = 0;
    }
#endif

    //
    // Also do some checking
    //
    if ((Desc->Attribute & EFI_MEMORY_RUNTIME) != 0) {
      //
      // block with RT flag.
      // if it is not RT or MMIO, then report to log
      //
      if (Desc->Type != EfiRuntimeServicesCode &&
        Desc->Type != EfiRuntimeServicesData &&
        Desc->Type != EfiMemoryMappedIO &&
        Desc->Type != EfiMemoryMappedIOPortSpace
        )
      {
        DEBUG ((DEBUG_VERBOSE, " %s with RT flag: %lx (0x%x) - ???\n", mEfiMemoryTypeDesc[Desc->Type], Desc->PhysicalStart, Desc->NumberOfPages));
      }
    } else {
      //
      // block without RT flag.
      // if it is RT or MMIO, then report to log
      //
      if (Desc->Type == EfiRuntimeServicesCode ||
        Desc->Type == EfiRuntimeServicesData ||
        Desc->Type == EfiMemoryMappedIO ||
        Desc->Type == EfiMemoryMappedIOPortSpace
        )
      {
        DEBUG ((DEBUG_VERBOSE, " %s without RT flag: %lx (0x%x) - ???\n", mEfiMemoryTypeDesc[Desc->Type], Desc->PhysicalStart, Desc->NumberOfPages));
      }
    }

    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
  }
}

/** Older boot.efi versions did not support too large memory maps, so we try to join BS_Code and BS_Data areas. */
VOID
EFIAPI
ShrinkMemMap (
  IN UINTN                    *MemoryMapSize,
  IN EFI_MEMORY_DESCRIPTOR    *MemoryMap,
  IN UINTN                    DescriptorSize,
  IN UINT32                   DescriptorVersion
  )
{
  UINTN                   SizeFromDescToEnd;
  UINT64                  Bytes;
  EFI_MEMORY_DESCRIPTOR   *PrevDesc;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  BOOLEAN                 CanBeJoined;
  BOOLEAN                 HasEntriesToRemove;

  PrevDesc           = MemoryMap;
  Desc               = NEXT_MEMORY_DESCRIPTOR(PrevDesc, DescriptorSize);
  SizeFromDescToEnd  = *MemoryMapSize - DescriptorSize;
  *MemoryMapSize     = DescriptorSize;
  HasEntriesToRemove = FALSE;

  while (SizeFromDescToEnd > 0) {
    Bytes = (((UINTN) PrevDesc->NumberOfPages) * EFI_PAGE_SIZE);
    CanBeJoined = FALSE;
    if (Desc->Attribute == PrevDesc->Attribute && PrevDesc->PhysicalStart + Bytes == Desc->PhysicalStart) {
      CanBeJoined = (Desc->Type == EfiBootServicesCode || Desc->Type == EfiBootServicesData) &&
        (PrevDesc->Type == EfiBootServicesCode || PrevDesc->Type == EfiBootServicesData);
    }

    if (CanBeJoined) {
      // two entries are the same/similar - join them
      PrevDesc->NumberOfPages += Desc->NumberOfPages;
      HasEntriesToRemove = TRUE;
    } else {
      // can not be joined - we need to move to next
      *MemoryMapSize += DescriptorSize;
      PrevDesc = NEXT_MEMORY_DESCRIPTOR(PrevDesc, DescriptorSize);
      if (HasEntriesToRemove) {
        // have entries between PrevDesc and Desc which are joined to PrevDesc
        // we need to copy [Desc, end of list] to PrevDesc + 1
        CopyMem(PrevDesc, Desc, SizeFromDescToEnd);
        Desc = PrevDesc;
        HasEntriesToRemove = FALSE;
      }
    }
    // move to next
    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
    SizeFromDescToEnd -= DescriptorSize;
  }
}

VOID
EFIAPI
PrintMemMap (
  IN UINTN                    MemoryMapSize,
  IN UINTN                    DescriptorSize,
  IN EFI_MEMORY_DESCRIPTOR    *MemoryMap
  )
{
  UINTN                   NumEntries;
  UINTN                   Index;
  UINT64                  Bytes;
  EFI_MEMORY_DESCRIPTOR   *Desc;

  Desc = MemoryMap;
  NumEntries = MemoryMapSize / DescriptorSize;
  Print(L"MEMMAP: Size=%d, Addr=%p, DescSize=%d\n", MemoryMapSize, MemoryMap, DescriptorSize);
  Print(L"Type      Start      End        Virtual          # Pages    Attributes\n");
  for (Index = 0; Index < NumEntries; Index++) {

    Bytes = EFI_PAGES_TO_SIZE(Desc->NumberOfPages);

    Print(L"%-9s %010lX %010lX %016lX %010lX %016lX\n",
      mEfiMemoryTypeDesc[Desc->Type],
      Desc->PhysicalStart,
      Desc->PhysicalStart + Bytes - 1,
      Desc->VirtualStart,
      Desc->NumberOfPages,
      Desc->Attribute
    );
    Desc = NEXT_MEMORY_DESCRIPTOR(Desc, DescriptorSize);
    if ((Index + 1) % 16 == 0)
      gBS->Stall(5000000);
  }
  //WaitForKeyPress(L"End: press a key to continue\n");
}

VOID
EFIAPI
PrintSystemTable (
  IN EFI_SYSTEM_TABLE  *ST
  )
{
#if !defined (MDEPKG_NDEBUG)
  UINTN  Index;

  DEBUG ((DEBUG_VERBOSE, "SysTable: %p\n", ST));
  DEBUG ((DEBUG_VERBOSE, "- FirmwareVendor: %p, %s\n", ST->FirmwareVendor, ST->FirmwareVendor));
  DEBUG ((DEBUG_VERBOSE, "- ConsoleInHandle: %p, ConIn: %p\n", ST->ConsoleInHandle, ST->ConIn));
  DEBUG ((DEBUG_VERBOSE, "- RuntimeServices: %p, BootServices: %p, ConfigurationTable: %p\n", ST->RuntimeServices, ST->BootServices, ST->ConfigurationTable));
  DEBUG ((DEBUG_VERBOSE, "RT:\n"));
  DEBUG ((DEBUG_VERBOSE, "- GetVariable: %p, SetVariable: %p\n", ST->RuntimeServices->GetVariable, ST->RuntimeServices->SetVariable));

  for(Index = 0; Index < ST->NumberOfTableEntries; Index++) {
    DEBUG ((DEBUG_VERBOSE, "ConfTab: %p\n", ST->ConfigurationTable[Index].VendorTable));
  }
#endif
}

VOID
WaitForKeyPress (
  CHAR16 *Message
  )
{
  EFI_STATUS      Status;
  UINTN           Index;
  EFI_INPUT_KEY   Key;

  Print(L"%a", Message);
  do {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  } while(Status == EFI_SUCCESS);
  gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
  do {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  } while(Status == EFI_SUCCESS);
}

/** Returns file path from FilePathProto in allocated memory. Mem should be released by caler.*/
CHAR16 *
EFIAPI
FileDevicePathToText (
  EFI_DEVICE_PATH_PROTOCOL *FilePathProto
  )
{
  FILEPATH_DEVICE_PATH    *FilePath;
  CHAR16                  FilePathText[256]; // possible problem: if filepath is bigger
  CHAR16                  *OutFilePathText;
  UINTN                   Size;
  UINTN                   SizeAll;
  UINTN                   i;

  FilePathText[0] = L'\0';
  i = 4;
  SizeAll = 0;
  //DEBUG ((DEBUG_VERBOSE, "FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto)));
  while (FilePathProto != NULL && FilePathProto->Type != END_DEVICE_PATH_TYPE && i > 0) {
    if (FilePathProto->Type == MEDIA_DEVICE_PATH && FilePathProto->SubType == MEDIA_FILEPATH_DP) {
      FilePath = (FILEPATH_DEVICE_PATH *) FilePathProto;
      Size = (DevicePathNodeLength(FilePathProto) - 4) / 2;
      if (SizeAll + Size < 256) {
        if (SizeAll > 0 && FilePathText[SizeAll / 2 - 2] != L'\\') {
          StrCatS(FilePathText, 256, L"\\");
        }
        StrCatS(FilePathText, 256, FilePath->PathName);
        SizeAll = StrSize(FilePathText);
      }
    }
    FilePathProto = NextDevicePathNode(FilePathProto);
    //DEBUG ((DEBUG_VERBOSE, "FilePathProto->Type: %d, SubType: %d, Length: %d\n", FilePathProto->Type, FilePathProto->SubType, DevicePathNodeLength(FilePathProto)));
    i--;
    //DEBUG ((DEBUG_VERBOSE, "FilePathText: %s\n", FilePathText));
  }

  OutFilePathText = NULL;
  Size = StrSize(FilePathText);
  if (Size > 2) {
    // we are allocating mem here - should be released by caller
    OutFilePathText = (CHAR16 *)DirectAllocatePool(Size);
    if (OutFilePathText)
      StrCpyS(OutFilePathText, Size/sizeof(CHAR16), FilePathText);
  }

  return OutFilePathText;
}

/** Helper function that calls GetMemoryMap(), allocates space for mem map and returns it. */
EFI_STATUS
EFIAPI
GetMemoryMapAlloc (
  IN EFI_GET_MEMORY_MAP           GetMemoryMapFunction,
  IN OUT UINTN                    *AllocatedTopPages,
  OUT UINTN                       *MemoryMapSize,
  OUT EFI_MEMORY_DESCRIPTOR       **MemoryMap,
  OUT UINTN                       *MapKey,
  OUT UINTN                       *DescriptorSize,
  OUT UINT32                      *DescriptorVersion
  )
{
  EFI_STATUS     Status;

  *MemoryMapSize = 0;
  *MemoryMap     = NULL;
  Status = GetMemoryMapFunction (
    MemoryMapSize,
    *MemoryMap,
    MapKey,
    DescriptorSize,
    DescriptorVersion
    );

  if (Status != EFI_BUFFER_TOO_SMALL) {
    DEBUG ((DEBUG_WARN, "Insane GetMemoryMap %r\n", Status));
    return Status;
  }

  do {
    // This is done because extra allocations may increase memory map size.
    *MemoryMapSize   += 256;

    // Requested to allocate from top via pages.
    // This may be needed, because the pool memory may collide with the kernel.
    if (AllocatedTopPages) {
      *MemoryMap         = (EFI_MEMORY_DESCRIPTOR *)BASE_4GB;
      *AllocatedTopPages = EFI_SIZE_TO_PAGES(*MemoryMapSize);
      Status = AllocatePagesFromTop (EfiBootServicesData, *AllocatedTopPages, (EFI_PHYSICAL_ADDRESS *)*MemoryMap);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_WARN, "Temp memory map allocation from top failure %r\n", Status));
        *MemoryMap = NULL;
        return Status;
      }
    } else {
      *MemoryMap = DirectAllocatePool (*MemoryMapSize);
      if (!*MemoryMap) {
        DEBUG ((DEBUG_WARN, "Temp memory map direct allocation failure\n"));
        return EFI_OUT_OF_RESOURCES;
      }
    }

    Status = GetMemoryMapFunction (
      MemoryMapSize,
      *MemoryMap,
      MapKey,
      DescriptorSize,
      DescriptorVersion
      );

    if (EFI_ERROR (Status)) {
      if (AllocatedTopPages)
        gBS->FreePages ((EFI_PHYSICAL_ADDRESS)*MemoryMap, *AllocatedTopPages);
      else
        DirectFreePool (*MemoryMap);
      *MemoryMap = NULL;
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  if (Status != EFI_SUCCESS)
    DEBUG ((DEBUG_WARN, "Failed to obtain memory map %r\n", Status));

  return Status;
}

/** Alloctes Pages from the top of mem, up to address specified in Memory. Returns allocated address in Memory. */
EFI_STATUS
EFIAPI
AllocatePagesFromTop (
  IN EFI_MEMORY_TYPE           MemoryType,
  IN UINTN                     Pages,
  IN OUT EFI_PHYSICAL_ADDRESS  *Memory
  )
{
  EFI_STATUS              Status;
  UINTN                   MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemoryMap;
  UINTN                   MapKey;
  UINTN                   DescriptorSize;
  UINT32                  DescriptorVersion;
  EFI_MEMORY_DESCRIPTOR   *MemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR   *Desc;

  Status = GetMemoryMapAlloc(gBS->GetMemoryMap, NULL, &MemoryMapSize, &MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = EFI_NOT_FOUND;

  MemoryMapEnd = NEXT_MEMORY_DESCRIPTOR(MemoryMap, MemoryMapSize);
  Desc = PREV_MEMORY_DESCRIPTOR(MemoryMapEnd, DescriptorSize);
  for ( ; Desc >= MemoryMap; Desc = PREV_MEMORY_DESCRIPTOR(Desc, DescriptorSize)) {

    if (Desc->Type == EfiConventionalMemory &&                          // free mem
      Pages <= Desc->NumberOfPages &&                                 // contains enough space
      Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Pages) <= *Memory) {    // contains space below specified Memory
      // free block found
      if (Desc->PhysicalStart + EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages) <= *Memory) {
        // the whole block is under Memory - allocate from the top of the block
        *Memory = Desc->PhysicalStart + EFI_PAGES_TO_SIZE((UINTN)Desc->NumberOfPages - Pages);
        //DEBUG ((DEBUG_VERBOSE, "found the whole block under top mem, allocating at %lx\n", *Memory));
      } else {
        // the block contains enough pages under Memory, but spans above it - allocate below Memory.
        *Memory = *Memory - EFI_PAGES_TO_SIZE(Pages);
        //DEBUG ((DEBUG_VERBOSE, "found the whole block under top mem, allocating at %lx\n", *Memory));
      }
      Status = gBS->AllocatePages(AllocateAddress,
                    MemoryType,
                    Pages,
                    Memory);
      //DEBUG ((DEBUG_VERBOSE, "Alloc Pages=%x, Addr=%lx, Status=%r\n", Pages, *Memory, Status));
      break;
    }
  }

  // release mem
  DirectFreePool(MemoryMap);

  return Status;
}

/** Calls real gBS->AllocatePool and returns pool memory. */
VOID *
DirectAllocatePool (
  UINTN     Size
  )
{
  EFI_STATUS         Status;
  VOID               *Buffer;
  EFI_ALLOCATE_POOL  Allocator;

  Allocator = gStoredAllocatePool ? gStoredAllocatePool : gBS->AllocatePool;

  Status = Allocator(EfiBootServicesData, Size, &Buffer);
  if (Status == EFI_SUCCESS)
    return Buffer;

  return NULL;
}

/** Calls real gBS->FreePool and frees pool memory. */
VOID
DirectFreePool (
  VOID      *Buffer
  )
{
  EFI_FREE_POOL Deallocator;

  Deallocator = gStoredFreePool ? gStoredFreePool : gBS->FreePool;

  Deallocator(Buffer);
}
