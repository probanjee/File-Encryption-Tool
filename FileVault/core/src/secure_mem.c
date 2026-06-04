/*
 * FileVault v2.0 — secure_mem.c
 * Platform-specific secure memory operations.
 */

#include "filevault/secure_mem.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#endif

void fv_secure_zero(void *ptr, size_t len) {
    if (!ptr || len == 0) {
        return;
    }

#if defined(_WIN32)
    /* Windows: SecureZeroMemory is guaranteed not to be optimized out */
    SecureZeroMemory(ptr, len);

#elif defined(__STDC_LIB_EXT1__) && __STDC_LIB_EXT1__ >= 201112L
    /* C11 Annex K: memset_s is guaranteed not to be optimized out */
    memset_s(ptr, len, 0, len);

#elif defined(__GNUC__) || defined(__clang__)
    /* GCC/Clang: explicit_bzero is guaranteed not to be optimized out */
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        explicit_bzero(ptr, len);
    #else
        /* Volatile pointer trick — compiler cannot prove the write is dead */
        memset(ptr, 0, len);
        __asm__ __volatile__("" : : "r"(ptr) : "memory");
    #endif

#else
    /* Portable fallback: volatile function pointer prevents optimization */
    {
        typedef void *(*memset_fn)(void *, int, size_t);
        static volatile memset_fn secure_memset = memset;
        secure_memset(ptr, 0, len);
    }
#endif
}

void *fv_secure_alloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    void *ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void fv_secure_free(void *ptr, size_t size) {
    if (ptr) {
        fv_secure_zero(ptr, size);
        free(ptr);
    }
}
