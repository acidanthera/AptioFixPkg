/**

  Hack configuration
  This is a gestalt of black art, do not edit.

  by vit9696

**/

#ifndef APTIOFIX_HACK_CONFIG_H
#define APTIOFIX_HACK_CONFIG_H

/**
 * Forces XNU to use old UEFI memory mapping after hibernation wake.
 * May cause memory corruption. See FixHibernateWakeWithoutRelocBlock for details.
 */
#ifndef APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP
#define APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP 1
#endif

/**
 * When attempting to reuse old UEFI memory mapping gBS->AllocatePool seems
 * to produce the same addresses way more often, and thus the system will not reboot
 * when accessing RTShims after waking from hibernation.
 * However, gBS->AllocatePool is dangerous, because it may overlap with the kernel
 * region and break aslr.
 */
#ifndef APTIOFIX_ALLOCATE_POOL_GIVES_STABLE_ADDR
#define APTIOFIX_ALLOCATE_POOL_GIVES_STABLE_ADDR APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP
#endif

/**
 * Attempt to protect certain CSM memory regions from being used by the kernel (by Slice).
 * On older firmwares this caused wake issues.
 */
#ifndef APTIOFIX_PROTECT_CSM_REGION
#define APTIOFIX_PROTECT_CSM_REGION 1
#endif

/** Calculate aslr slide ourselves when some addresses are not available for XNU. */
#ifndef APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION
#define APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION 1
#endif

/** This is important for several boards that cannot boot with slide=0, which safe mode enforces. */
#ifndef APTIOFIX_ALLOW_ASLR_IN_SAFE_MODE
#define APTIOFIX_ALLOW_ASLR_IN_SAFE_MODE 1
#endif

/** Hide slide=x value from os for increased security when using custom aslr. */
#ifndef APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT
#define APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION
#endif

/**
 * Speculated maximum kernel size (in bytes) to use when looking for a free memory region.
 * Used by APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION to determine valid slide values.
 * 10.12.6 allocates at least approximately 287 MBs, we round it to 384 MBs
 * This seems to work pretty well on X299. Yet it may be a good idea to make a boot-arg.
 */
#ifndef APTIOFIX_SPECULATED_KERNEL_SIZE
#define APTIOFIX_SPECULATED_KERNEL_SIZE ((UINTN)0x18000000)
#endif

/** Maximum number of supported runtime reloc protection areas */
#ifndef APTIFIX_MAX_RT_RELOC_NUM
#define APTIFIX_MAX_RT_RELOC_NUM ((UINTN)64)
#endif

/**
 * Perform invasive memory dumps when -aptiodump -v are passed to boot.efi.
 *  Fails some boots but allows to reliably get the memory maps (in-OS dtrace script is broken).
 *  Enable for development and testing purposes.
 */
#ifndef APTIOFIX_ALLOW_MEMORY_DUMP_ARG
#define APTIOFIX_ALLOW_MEMORY_DUMP_ARG 0
#endif

/**
 * Due to os crashes caused by using AllocatePool on several Skylake APTIO V boards we provide
 * a custom allocator which avoids the use of boot services allocator by preallocating a memory pool
 * and spreading it as requested throughout the boot process.
 * Another benefit is that this allocator will return higher memory and avoid conflicts with custom
 * ASLR implementation. On allocation failure the original allocator will be used.
 * This hack appears to cause launchd crashes at sleep wake on GA-X79-UP4. Since normally the bug
 * is not triggered by anything by -aptiodump, it is turned off by default. 
 */
#ifndef APTIOFIX_CUSTOM_POOL_ALLOCATOR
#define APTIOFIX_CUSTOM_POOL_ALLOCATOR APTIOFIX_ALLOW_MEMORY_DUMP_ARG
#endif

/**
 * Maximum reserved area used for a custom pool allocator.
 */
#ifndef APTIOFIX_CUSTOM_POOL_ALLOCATOR_SIZE
#define APTIOFIX_CUSTOM_POOL_ALLOCATOR_SIZE 0x20000000
#endif

#endif // APTIOFIX_HACK_CONFIG_H
