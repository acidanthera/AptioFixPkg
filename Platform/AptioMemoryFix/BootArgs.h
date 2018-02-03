/**

  Methods for finding, checking and fixing boot args

  by dmazar (defs from Clover)

**/

#ifndef APTIOFIX_BOOT_ARGS_H
#define APTIOFIX_BOOT_ARGS_H

/*
 * Video information...
 */

struct Boot_VideoV1 {
  UINT32  v_baseAddr; /* Base address of video memory */
  UINT32  v_display;  /* Display Code (if Applicable */
  UINT32  v_rowBytes; /* Number of bytes per pixel row */
  UINT32  v_width;    /* Width */
  UINT32  v_height;   /* Height */
  UINT32  v_depth;    /* Pixel Depth */
};
typedef struct Boot_VideoV1 Boot_VideoV1;

struct Boot_Video {
  UINT32 v_display;   /* Display Code (if Applicable */
  UINT32 v_rowBytes;  /* Number of bytes per pixel row */
  UINT32 v_width;     /* Width */
  UINT32 v_height;    /* Height */
  UINT32 v_depth;     /* Pixel Depth */
  UINT32 v_resv[7];   /* Reserved */
  UINT64 v_baseAddr;  /* Base address of video memory */
};
typedef struct Boot_Video Boot_Video;


/* Values for v_display */

#define GRAPHICS_MODE           1
#define FB_TEXT_MODE            2

/* Boot argument structure - passed into Mach kernel at boot time.
 * "Revision" can be incremented for compatible changes
 */
#define kBootArgsRevision       0
#define kBootArgsVersion        2

/* Snapshot constants of previous revisions that are supported */
#define kBootArgsVersion1       1
#define kBootArgsRevision1_4    4
#define kBootArgsRevision1_5    5
#define kBootArgsRevision1_6    6

#define kBootArgsVersion2       2
#define kBootArgsRevision2_0    0

#define kBootArgsEfiMode32      32
#define kBootArgsEfiMode64      64

/* Bitfields for boot_args->flags */
#define kBootArgsFlagRebootOnPanic   (1 << 0)
#define kBootArgsFlagHiDPI           (1 << 1)
#define kBootArgsFlagBlack           (1 << 2)
#define kBootArgsFlagCSRActiveConfig (1 << 3)
#define kBootArgsFlagCSRConfigMode   (1 << 4)
#define kBootArgsFlagCSRBoot         (1 << 5)
#define kBootArgsFlagBlackBg         (1 << 6)
#define kBootArgsFlagLoginUI         (1 << 7)
#define kBootArgsFlagInstallUI       (1 << 8)

#define BOOT_LINE_LENGTH   1024

/* version 1 before Lion */
typedef struct {
  UINT16          Revision;            /* Revision of boot_args structure */
  UINT16          Version;             /* Version of boot_args structure */

  CHAR8           CommandLine[BOOT_LINE_LENGTH]; /* Passed in command line */

  UINT32          MemoryMap;           /* Physical address of memory map */
  UINT32          MemoryMapSize;
  UINT32          MemoryMapDescriptorSize;
  UINT32          MemoryMapDescriptorVersion;

  Boot_VideoV1    Video;                /* Video Information */

  UINT32          deviceTreeP;          /* Physical address of flattened device tree */
  UINT32          deviceTreeLength;     /* Length of flattened tree */

  UINT32          kaddr;                /* Physical address of beginning of kernel text */
  UINT32          ksize;                /* Size of combined kernel text+data+efi */

  UINT32          efiRuntimeServicesPageStart;   /* physical address of defragmented runtime pages */
  UINT32          efiRuntimeServicesPageCount;
  UINT32          efiSystemTable;       /* physical address of system table in runtime area */

  UINT8           efiMode;              /* 32 = 32-bit, 64 = 64-bit */
  UINT8           __reserved1[3];
  UINT32          __reserved2[1];
  UINT32          performanceDataStart; /* physical address of log */
  UINT32          performanceDataSize;
  UINT64          efiRuntimeServicesVirtualPageStart; /* virtual address of defragmented runtime pages */
  UINT32          __reserved3[2];

} BootArgs1;

/* version2 as used in Lion, updated with High Sierra fields */
typedef struct {

  UINT16          Revision;        /* Revision of boot_args structure */
  UINT16          Version;         /* Version of boot_args structure */

  UINT8           efiMode;         /* 32 = 32-bit, 64 = 64-bit */
  UINT8           debugMode;       /* Bit field with behavior changes */
  UINT16          flags;

  CHAR8           CommandLine[BOOT_LINE_LENGTH]; /* Passed in command line */

  UINT32          MemoryMap;       /* Physical address of memory map */
  UINT32          MemoryMapSize;
  UINT32          MemoryMapDescriptorSize;
  UINT32          MemoryMapDescriptorVersion;

  Boot_VideoV1    VideoV1;            /* Video Information */

  UINT32          deviceTreeP;         /* Physical address of flattened device tree */
  UINT32          deviceTreeLength;    /* Length of flattened tree */

  UINT32          kaddr;               /* Physical address of beginning of kernel text */
  UINT32          ksize;               /* Size of combined kernel text+data+efi */

  UINT32          efiRuntimeServicesPageStart;        /* physical address of defragmented runtime pages */
  UINT32          efiRuntimeServicesPageCount;
  UINT64          efiRuntimeServicesVirtualPageStart; /* virtual address of defragmented runtime pages */

  UINT32          efiSystemTable;       /* physical address of system table in runtime area */
  UINT32          kslide;

  UINT32          performanceDataStart; /* physical address of log */
  UINT32          performanceDataSize;

  UINT32          keyStoreDataStart;    /* physical address of key store data */
  UINT32          keyStoreDataSize;
  UINT64          bootMemStart;         /* physical address of interpreter boot memory */
  UINT64          bootMemSize;
  UINT64          PhysicalMemorySize;
  UINT64          FSBFrequency;
  UINT64          pciConfigSpaceBaseAddress;
  UINT32          pciConfigSpaceStartBusNumber;
  UINT32          pciConfigSpaceEndBusNumber;
  UINT32          csrActiveConfig;
  UINT32          csrCapabilities;
  UINT32          boot_SMC_plimit;
  UINT16          bootProgressMeterStart;
  UINT16          bootProgressMeterEnd;
  Boot_Video      Video;                /* Video Information */

  UINT32          apfsDataStart;        /* Physical address of apfs volume key structure */
  UINT32          apfsDataSize;

  UINT32          __reserved4[710];
} BootArgs2;

/** Our internal structure to hold boot args params to make the code independent of the boot args version. */
typedef struct {
  UINT32  *MemoryMap;      /* We will change this value so we need pointer to original field. */
  UINT32  *MemoryMapSize;
  UINT32  *MemoryMapDescriptorSize;
  UINT32  *MemoryMapDescriptorVersion;

  CHAR8   *CommandLine;

  UINT32  *deviceTreeP;
  UINT32  *deviceTreeLength;

  UINT32  *csrActiveConfig;
} BootArguments;

BootArguments *
EFIAPI
GetBootArgs (
  VOID *bootArgs
  );

VOID *
EFIAPI
BootArgsFind (
  IN EFI_PHYSICAL_ADDRESS Start
  );

VOID
RemoveArgumentFromCommandLine (
  CHAR8        *CommandLine,
  CONST CHAR8  *Argument
  );

// Invalidate found boot arg if:
// - it is neither the beginning of Cmd, nor has space prefix            -> boot arg is a suffix of another arg
// - it has neither space suffix, nor \0 suffix, and does not end with = -> boot arg is a prefix of another arg
#define VERIFY_BOOT_ARG(Found, Cmd, Arg)                                       \
  do {                                                                         \
    UINTN Len = ARRAY_SIZE((Arg)) - 1;                                         \
    if (!(Found) || ((Found) != (Cmd) && *((Found) - 1) != ' ') ||             \
      ((Found)[Len] != ' ' && (Found)[Len] != '\0' && (Found)[Len-1] != '='))  \
        (Found) = NULL;                                                        \
  } while (0)

#endif // APTIOFIX_BOOT_ARGS_H
