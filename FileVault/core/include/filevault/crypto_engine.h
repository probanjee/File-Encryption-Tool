/*
 * FileVault v2.0 — Secure File Encryption Tool
 * crypto_engine.h — AES-256-GCM streaming encryption/decryption
 *
 * Provides the primary encrypt/decrypt API used by both CLI and GUI.
 * Uses OpenSSL EVP API for AES-256-GCM authenticated encryption.
 * Processes files in streaming 1MB chunks — never loads full file into RAM.
 */

#ifndef FILEVAULT_CRYPTO_ENGINE_H
#define FILEVAULT_CRYPTO_ENGINE_H

#include "filevault/errors.h"
#include "filevault/file_format.h"
#include <stdint.h>

/* Streaming chunk size: 1 MB */
#define FV_CHUNK_SIZE (1024 * 1024)

/* Progress callback — called after each chunk is processed */
typedef void (*FvProgressFn)(uint64_t bytes_done, uint64_t bytes_total,
                             void *user_data);

/*
 * Encryption request parameters.
 * The caller must provide input_path and password at minimum.
 * If output_path is NULL, auto-generates "input_path.vault".
 */
typedef struct {
    const char  *input_path;
    const char  *output_path;      /* NULL for auto-generate */
    const char  *password;
    int          force_overwrite;
    FvProgressFn progress_fn;
    void        *progress_data;
} FvEncryptRequest;

/*
 * Decryption request parameters.
 * The caller must provide input_path and password at minimum.
 * If output_path is NULL, auto-generates by stripping ".vault" + ".dec".
 * If verify_only is set, validates authentication without writing output.
 */
typedef struct {
    const char  *input_path;
    const char  *output_path;      /* NULL for auto-generate */
    const char  *password;
    int          force_overwrite;
    int          verify_only;
    FvProgressFn progress_fn;
    void        *progress_data;
} FvDecryptRequest;

/*
 * Operation result returned after encrypt/decrypt.
 */
typedef struct {
    FvError  error;
    char     input_path[1024];
    char     output_path[1024];
    uint64_t bytes_processed;
    double   duration_seconds;
} FvResult;

/*
 * Encrypt a file using AES-256-GCM.
 *
 * Flow:
 *   1. Validate input file
 *   2. Generate random 16-byte salt + 12-byte nonce
 *   3. Derive 256-bit key from password + salt via PBKDF2
 *   4. Write .vault header to output
 *   5. Stream-encrypt input in 1MB chunks
 *   6. Finalize and append 16-byte authentication tag
 *   7. Securely wipe all key material
 *
 * Returns FV_OK on success, or an error code.
 */
FvError fv_encrypt_file(const FvEncryptRequest *req, FvResult *result);

/*
 * Decrypt a .vault file using AES-256-GCM.
 *
 * Flow:
 *   1. Read and validate .vault header
 *   2. Extract salt, nonce from header
 *   3. Derive key from password + salt
 *   4. Read auth tag from end of file
 *   5. Stream-decrypt ciphertext to temp file
 *   6. Verify authentication tag via EVP_DecryptFinal_ex
 *   7. If valid: rename temp → final output
 *   8. If invalid: delete temp, return FV_ERR_AUTH_FAILED
 *   9. Securely wipe all key material
 *
 * Returns FV_OK on success, FV_ERR_AUTH_FAILED on wrong password/tamper.
 */
FvError fv_decrypt_file(const FvDecryptRequest *req, FvResult *result);

/*
 * Read file info without decrypting (for 'info' command).
 * Only reads and validates the header.
 */
FvError fv_file_info(const char *input_path, FvFileHeader *header);

#endif /* FILEVAULT_CRYPTO_ENGINE_H */
