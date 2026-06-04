/*
 * FileVault v2.0 — Secure File Encryption Tool
 * legacy_xor.h — Legacy XOR encryption mode (backward compatibility only)
 *
 * WARNING: XOR encryption is cryptographically insecure.
 * This module exists ONLY to decrypt files created by the original
 * File Encryption Tool v1.0. It should NOT be used for new files
 * in production environments.
 *
 * Bug fixes applied:
 *   - Key file path: "file.enc" → looks for "file.key" (not "file.enc.key")
 *   - Output name: "file.enc" → produces "file.dec" (not "file.enc.dec")
 *   - Empty key validation prevents divide-by-zero
 *   - Bounded input replaces unsafe scanf
 */

#ifndef FILEVAULT_LEGACY_XOR_H
#define FILEVAULT_LEGACY_XOR_H

#include "filevault/errors.h"
#include <stddef.h>
#include <stdint.h>

/* Maximum legacy key size (matches original globalKey[200]) */
#define FV_LEGACY_MAX_KEY 200

/* Chunk size for legacy streaming I/O */
#define FV_LEGACY_CHUNK_SIZE (64 * 1024)

/* Progress callback for legacy operations */
typedef void (*FvLegacyProgressFn)(uint64_t bytes_done, uint64_t bytes_total,
                                   void *user_data);

/*
 * Encrypt a file using legacy XOR mode.
 * Creates output_path with XOR-encrypted content.
 * Creates a .key file alongside the original file with the key.
 *
 * This function exists for testing legacy format compatibility.
 * A warning is printed to stderr.
 */
FvError fv_legacy_xor_encrypt(const char *input_path,
                              const char *output_path,
                              const char *key, size_t key_len,
                              FvLegacyProgressFn progress_fn,
                              void *progress_data);

/*
 * Decrypt a file using legacy XOR mode.
 * Automatically finds the key file by stripping ".enc" and appending ".key".
 * If key_file_path is provided, uses that instead of auto-detection.
 * If output_path is NULL, auto-generates by stripping ".enc" + appending ".dec".
 */
FvError fv_legacy_xor_decrypt(const char *input_path,
                              const char *key_file_path,
                              const char *output_path,
                              FvLegacyProgressFn progress_fn,
                              void *progress_data);

/*
 * Encrypt a file using legacy XOR, auto-generating output and key paths.
 * input_path → input_path.enc (encrypted data)
 * input_path → input_path.key (plaintext key — legacy behavior)
 */
FvError fv_legacy_xor_encrypt_auto(const char *input_path,
                                   const char *key, size_t key_len,
                                   FvLegacyProgressFn progress_fn,
                                   void *progress_data);

#endif /* FILEVAULT_LEGACY_XOR_H */
