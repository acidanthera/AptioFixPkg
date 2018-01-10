/**

  Hack configuration
  This is a gestalt of black art, do not edit.

  by vit9696

**/

#ifndef APTIOFIX_HACK_CONFIG_H
#define APTIOFIX_HACK_CONFIG_H

/** Forces XNU to use old UEFI memory mapping after hibernation wake.
 *  May cause memory corruption. See FixHibernateWakeWithoutRelocBlock for details.
 */
#define APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP 1

/** When attempting to reuse old UEFI memory mapping gBS->AllocatePool seems
 *  to produce the same addresses way more often, and thus the system will not reboot
 *  when accessing RTShims after waking from hibernation.
 *  However, gBS->AllocatePool is dangerous, because it may overlap with the kernel
 *  region and break aslr.
 */
#define APTIOFIX_ALLOCATE_POOL_GIVES_STABLE_ADDR APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP

/** Assign virtual addresses to a reserved IGPU area.
 *  It is believed that smth accesses this area via virtual addresses if it is mapped.
 *  Previous fix avoided this memory from being mapped at all.
 */
#define APTIOFIX_PROTECT_IGPU_RESERVED_MEMORY 1

/**  Attempt to protect some memory region from being used by the kernel.
 *   After we disabled memory relocation this should no longer be needed.
 *   Left for historical reasons, do NOT use!!!
 */
#define APTIOFIX_SLICE_OVERLAPPING_REGION_FIX 0

/** Calculate aslr slide ourselves when some addresses are not available for XNU. */
#define APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION 1

/** This is important for several boards that cannot boot with slide=0, which safe mode enforces. */
#define APTIOFIX_ALLOW_ASLR_IN_SAFE_MODE 1

/** Hide slide=x value from os for increased security when using custom aslr. */
#define APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION

/** Speculated maximum kernel size (in bytes) to use when looking for a free memory region.
 *  Used by APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION to determine valid slide values.
 *  10.12.6 allocates at least approximately 287 MBs, we round it to 384 MBs
 *  This seems to work pretty well on X299. Yet it may be a good idea to make a boot-arg.
 */
#define APTIOFIX_SPECULATED_KERNEL_SIZE ((UINTN)0x18000000)

/** Maximum number of supported runtime reloc protection areas */
#define APTIFIX_MAX_RT_RELOC_NUM ((UINTN)64)

/** Perform invasive memory dumps when -aptiodump -v are passed to boot.efi.
 *  Fails some boots but allows to reliably get the memory maps (in-OS dtrace script is broken).
 *  Enable for development and testing purposes.
 */
#define APTIOFIX_ALLOW_MEMORY_DUMP_ARG 0

#endif // APTIOFIX_HACK_CONFIG_H
