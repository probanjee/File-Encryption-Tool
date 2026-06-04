/*
 * FileVault v2.0 — Secure File Encryption Tool
 * secure_mem.h — Secure memory operations
 *
 * Platform-specific secure memory wipe to prevent sensitive data
 * (passwords, keys, salts, nonces) from lingering in memory.
 */

#ifndef FILEVAULT_SECURE_MEM_H
#define FILEVAULT_SECURE_MEM_H

#include <stddef.h>

/*
 * Securely zero a memory region. Guaranteed not to be optimized away.
 * Uses SecureZeroMemory on Windows, explicit_bzero on Linux/BSD,
 * and a volatile-pointer fallback on other platforms.
 */
void fv_secure_zero(void *ptr, size_t len);

/*
 * Allocate memory and zero it. Returns NULL on failure.
 */
void *fv_secure_alloc(size_t size);

/*
 * Securely zero and free a memory region.
 * ptr may be NULL (no-op).
 */
void fv_secure_free(void *ptr, size_t size);

#endif /* FILEVAULT_SECURE_MEM_H */
