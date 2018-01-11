/**

  Methods for finding, checking and fixing boot args

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>

#include "Config.h"
#include "BootArgs.h"
#include "Lib.h"

BootArgs gBootArgs;

BootArgs *
EFIAPI
GetBootArgs (
	VOID *bootArgs
	)
{
	BootArgs1  *BA1 = bootArgs;
	BootArgs2  *BA2 = bootArgs;

	ZeroMem(&gBootArgs, sizeof(gBootArgs));

	if (BA1->Version == kBootArgsVersion1) {
		// pre Lion
		gBootArgs.MemoryMap = &BA1->MemoryMap;
		gBootArgs.MemoryMapSize = &BA1->MemoryMapSize;
		gBootArgs.MemoryMapDescriptorSize = &BA1->MemoryMapDescriptorSize;
		gBootArgs.MemoryMapDescriptorVersion = &BA1->MemoryMapDescriptorVersion;

		gBootArgs.CommandLine = &BA1->CommandLine[0];

		gBootArgs.deviceTreeP = &BA1->deviceTreeP;
		gBootArgs.deviceTreeLength = &BA1->deviceTreeLength;

		gBootArgs.kaddr = &BA1->kaddr;
		gBootArgs.ksize = &BA1->ksize;

		gBootArgs.efiRuntimeServicesPageStart = &BA1->efiRuntimeServicesPageStart;
		gBootArgs.efiRuntimeServicesPageCount = &BA1->efiRuntimeServicesPageCount;
		gBootArgs.efiRuntimeServicesVirtualPageStart = &BA1->efiRuntimeServicesVirtualPageStart;
		gBootArgs.efiSystemTable = &BA1->efiSystemTable;

		gBootArgs.csrActiveConfig = NULL;
	} else {
		// Lion and up
		gBootArgs.MemoryMap = &BA2->MemoryMap;
		gBootArgs.MemoryMapSize = &BA2->MemoryMapSize;
		gBootArgs.MemoryMapDescriptorSize = &BA2->MemoryMapDescriptorSize;
		gBootArgs.MemoryMapDescriptorVersion = &BA2->MemoryMapDescriptorVersion;

		gBootArgs.CommandLine = &BA2->CommandLine[0];

		gBootArgs.deviceTreeP = &BA2->deviceTreeP;
		gBootArgs.deviceTreeLength = &BA2->deviceTreeLength;

		gBootArgs.kaddr = &BA2->kaddr;
		gBootArgs.ksize = &BA2->ksize;

		gBootArgs.efiRuntimeServicesPageStart = &BA2->efiRuntimeServicesPageStart;
		gBootArgs.efiRuntimeServicesPageCount = &BA2->efiRuntimeServicesPageCount;
		gBootArgs.efiRuntimeServicesVirtualPageStart = &BA2->efiRuntimeServicesVirtualPageStart;
		gBootArgs.efiSystemTable = &BA2->efiSystemTable;

		if (BA2->flags & kBootArgsFlagCSRActiveConfig) {
			gBootArgs.csrActiveConfig = &BA2->csrActiveConfig;
		} else {
			gBootArgs.csrActiveConfig = NULL;
		}
	}

	return &gBootArgs;
}

/** Searches for bootArgs from Start and returns pointer to bootArgs or ... does not return if can not be found.  **/
VOID *
EFIAPI
BootArgsFind (
	IN EFI_PHYSICAL_ADDRESS Start
	)
{
	UINT8        *ptr;
	UINT8        archMode = sizeof(UINTN) * 8;
	BootArgs1    *BA1;
	BootArgs2    *BA2;

	// start searching from 0x200000.
	ptr = (UINT8*)(UINTN)Start;

	while(TRUE) {

		// check bootargs for 10.7 and up
		BA2 = (BootArgs2*)ptr;

		if (BA2->Version==2 && BA2->Revision==0
			// plus additional checks - some values are not inited by boot.efi yet
			&& BA2->efiMode == archMode
			&& BA2->kaddr == 0 && BA2->ksize == 0
			&& BA2->efiSystemTable == 0
			)
		{
			break;
		}

		// check bootargs for 10.4 - 10.6.x
		BA1 = (BootArgs1*)ptr;

		if (BA1->Version==1
			&& (BA1->Revision==6 || BA1->Revision==5 || BA1->Revision==4)
			// plus additional checks - some values are not inited by boot.efi yet
			&& BA1->efiMode == archMode
			&& BA1->kaddr == 0 && BA1->ksize == 0
			&& BA1->efiSystemTable == 0
			)
		{
			break;
		}

		ptr += 0x1000;
	}

	DEBUG ((DEBUG_VERBOSE, "Found bootArgs at %p\n", ptr));
	return ptr;
}

VOID
RemoveArgumentFromCommandLine (
	CHAR8        *CommandLine,
	CONST CHAR8  *Argument
	)
{
	CHAR8 *Match = NULL;

	do {
		Match = AsciiStrStr (CommandLine, Argument);
		if (Match && (Match == CommandLine || *(Match - 1) == ' ')) {
			while (*Match != ' ' && *Match != '\0')
				*Match++ = ' ';
		}
	} while (Match != NULL);

	// Write zeroes to reduce data leak
	CHAR8 *Updated = CommandLine;

	while (CommandLine[0] == ' ')
		CommandLine++;

	while (CommandLine[0] != '\0') {
		while (CommandLine[0] == ' ' && CommandLine[1] == ' ')
			CommandLine++;

		*Updated++ = *CommandLine++;
	}

	while (Updated != CommandLine)
		*Updated++ = '\0';
}
