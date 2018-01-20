/**

  System Integrity Proteciton codes.

  by vit9696

**/

#ifndef APTIOFIX_CSR_CONFIG_H
#define APTIOFIX_CSR_CONFIG_H

/* Rootless configuration flags (xnu/bsd/sys/csr.h) */
#define CSR_ALLOW_UNTRUSTED_KEXTS             (1 << 0)
#define CSR_ALLOW_UNRESTRICTED_FS             (1 << 1)
#define CSR_ALLOW_TASK_FOR_PID                (1 << 2)
#define CSR_ALLOW_KERNEL_DEBUGGER             (1 << 3)
#define CSR_ALLOW_APPLE_INTERNAL              (1 << 4)
#define CSR_ALLOW_DESTRUCTIVE_DTRACE          (1 << 5) /* name deprecated */
#define CSR_ALLOW_UNRESTRICTED_DTRACE         (1 << 5)
#define CSR_ALLOW_UNRESTRICTED_NVRAM          (1 << 6)
#define CSR_ALLOW_DEVICE_CONFIGURATIO         (1 << 7)
#define CSR_ALLOW_ANY_RECOVERY_OS             (1 << 8)
#define CSR_ALLOW_UNAPPROVED_KEXTS            (1 << 9)

#endif // APTIOFIX_CSR_CONFIG_H
