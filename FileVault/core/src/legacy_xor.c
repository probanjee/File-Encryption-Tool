/*
 * FileVault v2.0 — legacy_xor.c
 * Legacy XOR encryption/decryption with all original bugs fixed.
 *
 * This is the cleaned-up version of the original encryption.c / decryption.c.
 * Changes from original:
 *   1. Key file lookup: strips ".enc" before appending ".key"
 *   2. Output naming: strips ".enc" before appending ".dec"
 *   3. Empty key check prevents divide-by-zero
 *   4. Uses fgets instead of scanf
 *   5. Output file created only after all validation passes
 *   6. Streaming I/O with chunks instead of byte-by-byte fgetc
 *   7. Secure memory wipe of key after use
 */

#include "filevault/legacy_xor.h"
#include "filevault/file_utils.h"
#include "filevault/secure_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_legacy_warning(void) {
    fprintf(stderr,
        "\n"
        "  [WARN] Legacy XOR mode is cryptographically insecure.\n"
        "  [WARN] This mode exists only for backward compatibility.\n"
        "  [WARN] Use 'filevault encrypt' for secure AES-256-GCM encryption.\n"
        "\n");
}

static FvError xor_process_file(const char *input_path,
                                const char *output_path,
                                const char *key, size_t key_len,
                                FvLegacyProgressFn progress_fn,
                                void *progress_data) {
    FILE *in = NULL;
    FILE *out = NULL;
    unsigned char *buf = NULL;
    FvError result = FV_OK;

    /* Validate key */
    if (!key || key_len == 0) {
        return FV_ERR_EMPTY_PASSWORD;
    }

    in = fopen(input_path, "rb");
    if (!in) {
        return FV_ERR_FILE_OPEN;
    }

    out = fopen(output_path, "wb");
    if (!out) {
        fclose(in);
        return FV_ERR_FILE_OPEN;
    }

    /* Get total size for progress reporting */
    int64_t total_size = fv_file_size(input_path);
    uint64_t bytes_done = 0;

    /* Allocate chunk buffer */
    buf = (unsigned char *)malloc(FV_LEGACY_CHUNK_SIZE);
    if (!buf) {
        result = FV_ERR_MEMORY;
        goto cleanup;
    }

    /* Process in chunks — never loads entire file into RAM */
    size_t bytes_read;
    size_t key_offset = 0;

    while ((bytes_read = fread(buf, 1, FV_LEGACY_CHUNK_SIZE, in)) > 0) {
        /* XOR each byte with the key (cyclic) */
        for (size_t i = 0; i < bytes_read; i++) {
            buf[i] ^= (unsigned char)key[key_offset % key_len];
            key_offset++;
        }

        size_t written = fwrite(buf, 1, bytes_read, out);
        if (written != bytes_read) {
            result = FV_ERR_FILE_WRITE;
            goto cleanup;
        }

        bytes_done += bytes_read;

        if (progress_fn && total_size > 0) {
            progress_fn(bytes_done, (uint64_t)total_size, progress_data);
        }
    }

    if (ferror(in)) {
        result = FV_ERR_FILE_READ;
    }

cleanup:
    if (buf) {
        fv_secure_zero(buf, FV_LEGACY_CHUNK_SIZE);
        free(buf);
    }
    if (out) fclose(out);
    if (in) fclose(in);

    /* If we failed, remove partial output */
    if (result != FV_OK) {
        remove(output_path);
    }

    return result;
}

FvError fv_legacy_xor_encrypt(const char *input_path,
                              const char *output_path,
                              const char *key, size_t key_len,
                              FvLegacyProgressFn progress_fn,
                              void *progress_data) {
    if (!input_path || !output_path || !key) {
        return FV_ERR_INVALID_ARGS;
    }

    print_legacy_warning();

    /* Validate input */
    FvError err = fv_validate_input_path(input_path);
    if (err != FV_OK) return err;

    /* Validate key */
    if (key_len == 0) {
        return FV_ERR_EMPTY_PASSWORD;
    }

    /* Validate output */
    err = fv_validate_output_path(output_path, 0);
    if (err != FV_OK && err != FV_ERR_OUTPUT_EXISTS) return err;

    return xor_process_file(input_path, output_path, key, key_len,
                            progress_fn, progress_data);
}

FvError fv_legacy_xor_decrypt(const char *input_path,
                              const char *key_file_path,
                              const char *output_path,
                              FvLegacyProgressFn progress_fn,
                              void *progress_data) {
    if (!input_path) {
        return FV_ERR_INVALID_ARGS;
    }

    print_legacy_warning();

    /* Validate input file */
    FvError err = fv_validate_input_path(input_path);
    if (err != FV_OK) return err;

    /* Resolve key file path */
    char resolved_key_path[FV_MAX_PATH];
    if (key_file_path && key_file_path[0] != '\0') {
        if (strlen(key_file_path) >= FV_MAX_PATH) {
            return FV_ERR_FILENAME_TOO_LONG;
        }
        memcpy(resolved_key_path, key_file_path, strlen(key_file_path) + 1);
    } else {
        /*
         * BUG FIX: Auto-detect key file.
         * Original bug: "King.txt.enc" → looked for "King.txt.enc.key"
         * Fixed: strips ".enc" → "King.txt" → appends ".key" → "King.txt.key"
         */
        err = fv_generate_legacy_key_path(input_path, resolved_key_path,
                                          sizeof(resolved_key_path));
        if (err != FV_OK) return err;
    }

    /* Read key from key file */
    if (!fv_file_exists(resolved_key_path)) {
        fprintf(stderr, "  [ERROR] Key file not found: %s\n", resolved_key_path);
        return FV_ERR_FILE_NOT_FOUND;
    }

    FILE *key_file = fopen(resolved_key_path, "r");
    if (!key_file) {
        return FV_ERR_FILE_OPEN;
    }

    char key_buf[FV_LEGACY_MAX_KEY];
    memset(key_buf, 0, sizeof(key_buf));

    /* Safe read — replaces original fscanf(keyFile, "%s", globalKey) */
    err = fv_safe_read_line(key_buf, sizeof(key_buf), key_file);
    fclose(key_file);

    if (err != FV_OK) {
        fv_secure_zero(key_buf, sizeof(key_buf));
        return err;
    }

    size_t key_len = strlen(key_buf);

    /* BUG FIX: Prevent divide-by-zero when key is empty */
    if (key_len == 0) {
        fprintf(stderr, "  [ERROR] Key file is empty: %s\n", resolved_key_path);
        fv_secure_zero(key_buf, sizeof(key_buf));
        return FV_ERR_EMPTY_PASSWORD;
    }

    /* Resolve output path */
    char resolved_output[FV_MAX_PATH];
    if (output_path && output_path[0] != '\0') {
        if (strlen(output_path) >= FV_MAX_PATH) {
            fv_secure_zero(key_buf, sizeof(key_buf));
            return FV_ERR_FILENAME_TOO_LONG;
        }
        memcpy(resolved_output, output_path, strlen(output_path) + 1);
    } else {
        /*
         * BUG FIX: Auto-generate output path.
         * Original bug: "King.txt.enc" → "King.txt.enc.dec"
         * Fixed: strips ".enc" → "King.txt" → appends ".dec" → "King.txt.dec"
         */
        err = fv_generate_legacy_decrypt_output(input_path, resolved_output,
                                                sizeof(resolved_output));
        if (err != FV_OK) {
            fv_secure_zero(key_buf, sizeof(key_buf));
            return err;
        }
    }

    /* Process file */
    FvError result = xor_process_file(input_path, resolved_output,
                                      key_buf, key_len,
                                      progress_fn, progress_data);

    /* Secure wipe of key from memory */
    fv_secure_zero(key_buf, sizeof(key_buf));

    return result;
}

FvError fv_legacy_xor_encrypt_auto(const char *input_path,
                                   const char *key, size_t key_len,
                                   FvLegacyProgressFn progress_fn,
                                   void *progress_data) {
    if (!input_path || !key) return FV_ERR_INVALID_ARGS;
    if (key_len == 0) return FV_ERR_EMPTY_PASSWORD;

    /* Generate output path: input.enc */
    char output_path[FV_MAX_PATH];
    FvError err = fv_generate_legacy_encrypt_output(input_path, output_path,
                                                    sizeof(output_path));
    if (err != FV_OK) return err;

    /* Generate key file path: input.key */
    char key_path[FV_MAX_PATH];
    size_t needed = strlen(input_path) + 5;
    if (needed > sizeof(key_path)) return FV_ERR_FILENAME_TOO_LONG;
    snprintf(key_path, sizeof(key_path), "%s.key", input_path);

    print_legacy_warning();

    /* Validate input */
    err = fv_validate_input_path(input_path);
    if (err != FV_OK) return err;

    /* Encrypt the file */
    err = xor_process_file(input_path, output_path, key, key_len,
                           progress_fn, progress_data);
    if (err != FV_OK) return err;

    /* Save key file (legacy behavior — plaintext key) */
    FILE *kf = fopen(key_path, "w");
    if (!kf) {
        remove(output_path);
        return FV_ERR_FILE_OPEN;
    }
    fprintf(kf, "%.*s", (int)key_len, key);
    fclose(kf);

    return FV_OK;
}
