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
#ifndef APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP
#define APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP 1
#endif

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
#ifndef APTIOFIX_PROTECT_IGPU_RESERVED_MEMORY
#define APTIOFIX_PROTECT_IGPU_RESERVED_MEMORY 1
#endif

/** It is believed that boot.efi on Sandy & Ivy skips 0x10200000 bytes from 0x10000000
 *  to protect from IGPU bugs, yet if this memory is marked available, it will may be
 *  used by XNU. So far attempts to enable this did not show any pattern but boot failures.
 */
#ifndef APTIOFIX_PROTECT_IGPU_SANDY_IVY_RESERVED_MEMORY
#define APTIOFIX_PROTECT_IGPU_SANDY_IVY_RESERVED_MEMORY 0
#endif

/** Attempt to protect some memory region from being used by the kernel (by Slice).
 *  It is believed to cause sleep issues on some systems, because this region
 *  is generally marked as conventional memory.
 */
#ifndef APTIOFIX_UNMARKED_OVERLAPPING_REGION_FIX
#define APTIOFIX_UNMARKED_OVERLAPPING_REGION_FIX 1
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
#define APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION

/** Speculated maximum kernel size (in bytes) to use when looking for a free memory region.
 *  Used by APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION to determine valid slide values.
 *  10.12.6 allocates at least approximately 287 MBs, we round it to 384 MBs
 *  This seems to work pretty well on X299. Yet it may be a good idea to make a boot-arg.
 */
#ifndef APTIOFIX_SPECULATED_KERNEL_SIZE
#define APTIOFIX_SPECULATED_KERNEL_SIZE ((UINTN)0x18000000)
#endif

/** Maximum number of supported runtime reloc protection areas */
#ifndef APTIFIX_MAX_RT_RELOC_NUM
#define APTIFIX_MAX_RT_RELOC_NUM ((UINTN)64)
#endif

/** Perform invasive memory dumps when -aptiodump -v are passed to boot.efi.
 *  Fails some boots but allows to reliably get the memory maps (in-OS dtrace script is broken).
 *  Enable for development and testing purposes.
 */
#ifndef APTIOFIX_ALLOW_MEMORY_DUMP_ARG
#define APTIOFIX_ALLOW_MEMORY_DUMP_ARG 0
#endif

#endif // APTIOFIX_HACK_CONFIG_H
