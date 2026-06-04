/*
 * FileVault v2.0 — crypto_engine.c
 * AES-256-GCM streaming encryption and decryption using OpenSSL EVP API.
 *
 * Security design:
 *   - Each encryption generates a fresh random salt + nonce
 *   - Key derived via PBKDF2-HMAC-SHA256 (600,000 iterations)
 *   - 1MB streaming chunks — never loads full file into RAM
 *   - Decryption writes to temp file, renames only after auth tag verifies
 *   - All key material securely wiped after use
 */

#include "filevault/crypto_engine.h"
#include "filevault/password.h"
#include "filevault/file_utils.h"
#include "filevault/secure_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <time.h>
#endif

/* High-resolution timer for duration measurement */
static double get_time_seconds(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#endif
}

FvError fv_encrypt_file(const FvEncryptRequest *req, FvResult *result) {
    if (!req || !req->input_path || !req->password) {
        return FV_ERR_INVALID_ARGS;
    }

    FvError err;
    FILE *fin = NULL, *fout = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char *chunk_in = NULL;
    unsigned char *chunk_out = NULL;
    unsigned char key[FV_KEY_SIZE];
    unsigned char salt[FV_SALT_SIZE];
    unsigned char nonce[FV_NONCE_SIZE];
    unsigned char tag[FV_TAG_SIZE];

    memset(key, 0, sizeof(key));
    memset(salt, 0, sizeof(salt));
    memset(nonce, 0, sizeof(nonce));
    memset(tag, 0, sizeof(tag));

    if (result) {
        memset(result, 0, sizeof(FvResult));
    }

    double start_time = get_time_seconds();

    /* Validate password */
    size_t password_len = strlen(req->password);
    err = fv_validate_password(req->password, password_len);
    if (err != FV_OK) goto cleanup;

    /* Validate input */
    err = fv_validate_input_path(req->input_path);
    if (err != FV_OK) goto cleanup;

    /* Resolve output path */
    char output_path[FV_MAX_PATH];
    if (req->output_path && req->output_path[0] != '\0') {
        if (strlen(req->output_path) >= FV_MAX_PATH) {
            err = FV_ERR_FILENAME_TOO_LONG;
            goto cleanup;
        }
        snprintf(output_path, sizeof(output_path), "%s", req->output_path);
    } else {
        err = fv_generate_encrypt_output(req->input_path, output_path,
                                         sizeof(output_path));
        if (err != FV_OK) goto cleanup;
    }

    /* Validate output */
    err = fv_validate_output_path(output_path, req->force_overwrite);
    if (err != FV_OK) goto cleanup;

    /* Generate random salt and nonce */
    err = fv_generate_random(salt, FV_SALT_SIZE);
    if (err != FV_OK) goto cleanup;

    err = fv_generate_random(nonce, FV_NONCE_SIZE);
    if (err != FV_OK) goto cleanup;

    /* Derive encryption key from password + salt */
    err = fv_derive_key(req->password, password_len,
                        salt, FV_SALT_SIZE,
                        key, FV_KEY_SIZE);
    if (err != FV_OK) goto cleanup;

    /* Build header */
    FvFileHeader header;
    fv_header_init(&header, salt, FV_SALT_SIZE, nonce, FV_NONCE_SIZE,
                   req->input_path);

    /* Open input file */
    fin = fopen(req->input_path, "rb");
    if (!fin) {
        err = FV_ERR_FILE_OPEN;
        goto cleanup;
    }

    /* Open output file */
    fout = fopen(output_path, "wb");
    if (!fout) {
        err = FV_ERR_FILE_OPEN;
        goto cleanup;
    }

    /* Write header */
    err = fv_write_header(fout, &header);
    if (err != FV_OK) goto cleanup;

    /* Initialize AES-256-GCM encryption */
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    /* Set nonce length (12 bytes is default for GCM, but be explicit) */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, FV_NONCE_SIZE, NULL) != 1) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce) != 1) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    /* Allocate chunk buffers */
    chunk_in = (unsigned char *)malloc(FV_CHUNK_SIZE);
    /* GCM output can be at most input_size + block_size - 1 */
    chunk_out = (unsigned char *)malloc(FV_CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH);
    if (!chunk_in || !chunk_out) {
        err = FV_ERR_MEMORY;
        goto cleanup;
    }

    /* Get file size for progress */
    int64_t total_size = fv_file_size(req->input_path);
    uint64_t bytes_done = 0;

    /* Stream-encrypt in chunks */
    size_t bytes_read;
    int out_len;

    while ((bytes_read = fread(chunk_in, 1, FV_CHUNK_SIZE, fin)) > 0) {
        if (EVP_EncryptUpdate(ctx, chunk_out, &out_len,
                              chunk_in, (int)bytes_read) != 1) {
            err = FV_ERR_ENCRYPT_FAILED;
            goto cleanup;
        }

        if (out_len > 0) {
            if (fwrite(chunk_out, 1, (size_t)out_len, fout) != (size_t)out_len) {
                err = FV_ERR_FILE_WRITE;
                goto cleanup;
            }
        }

        bytes_done += bytes_read;

        if (req->progress_fn && total_size > 0) {
            req->progress_fn(bytes_done, (uint64_t)total_size,
                             req->progress_data);
        }
    }

    if (ferror(fin)) {
        err = FV_ERR_FILE_READ;
        goto cleanup;
    }

    /* Finalize encryption */
    if (EVP_EncryptFinal_ex(ctx, chunk_out, &out_len) != 1) {
        err = FV_ERR_ENCRYPT_FAILED;
        goto cleanup;
    }

    if (out_len > 0) {
        if (fwrite(chunk_out, 1, (size_t)out_len, fout) != (size_t)out_len) {
            err = FV_ERR_FILE_WRITE;
            goto cleanup;
        }
    }

    /* Get authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, FV_TAG_SIZE, tag) != 1) {
        err = FV_ERR_ENCRYPT_FAILED;
        goto cleanup;
    }

    /* Append tag at end of file */
    if (fwrite(tag, 1, FV_TAG_SIZE, fout) != FV_TAG_SIZE) {
        err = FV_ERR_FILE_WRITE;
        goto cleanup;
    }

    err = FV_OK;

    /* Fill result */
    if (result) {
        result->error = FV_OK;
        snprintf(result->input_path, sizeof(result->input_path),
                 "%s", req->input_path);
        snprintf(result->output_path, sizeof(result->output_path),
                 "%s", output_path);
        result->bytes_processed = bytes_done;
        result->duration_seconds = get_time_seconds() - start_time;
    }

cleanup:
    /* Securely wipe all sensitive material */
    fv_secure_zero(key, sizeof(key));
    fv_secure_zero(salt, sizeof(salt));
    fv_secure_zero(nonce, sizeof(nonce));
    fv_secure_zero(tag, sizeof(tag));

    if (chunk_in) {
        fv_secure_zero(chunk_in, FV_CHUNK_SIZE);
        free(chunk_in);
    }
    if (chunk_out) {
        fv_secure_zero(chunk_out, FV_CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH);
        free(chunk_out);
    }
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (fin) fclose(fin);
    if (fout) fclose(fout);

    /* Remove partial output on failure */
    if (err != FV_OK) {
        remove(output_path);
        if (result) {
            result->error = err;
        }
    }

    return err;
}

FvError fv_decrypt_file(const FvDecryptRequest *req, FvResult *result) {
    if (!req || !req->input_path || !req->password) {
        return FV_ERR_INVALID_ARGS;
    }

    FvError err;
    FILE *fin = NULL, *fout = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char *chunk_in = NULL;
    unsigned char *chunk_out = NULL;
    unsigned char key[FV_KEY_SIZE];
    unsigned char tag[FV_TAG_SIZE];
    char temp_path[FV_MAX_PATH];
    char output_path[FV_MAX_PATH];
    int temp_file_created = 0;

    memset(key, 0, sizeof(key));
    memset(tag, 0, sizeof(tag));
    memset(temp_path, 0, sizeof(temp_path));
    memset(output_path, 0, sizeof(output_path));

    if (result) {
        memset(result, 0, sizeof(FvResult));
    }

    double start_time = get_time_seconds();

    /* Validate password */
    size_t password_len = strlen(req->password);
    if (password_len == 0) {
        err = FV_ERR_EMPTY_PASSWORD;
        goto cleanup;
    }

    /* Validate input */
    err = fv_validate_input_path(req->input_path);
    if (err != FV_OK) goto cleanup;

    /* Open input and read header */
    fin = fopen(req->input_path, "rb");
    if (!fin) {
        err = FV_ERR_FILE_OPEN;
        goto cleanup;
    }

    FvFileHeader header;
    err = fv_read_header(fin, &header);
    if (err != FV_OK) goto cleanup;

    err = fv_validate_header(&header);
    if (err != FV_OK) goto cleanup;

    /* Resolve output path */
    if (req->output_path && req->output_path[0] != '\0') {
        if (strlen(req->output_path) >= FV_MAX_PATH) {
            err = FV_ERR_FILENAME_TOO_LONG;
            goto cleanup;
        }
        snprintf(output_path, sizeof(output_path), "%s", req->output_path);
    } else {
        err = fv_generate_decrypt_output(req->input_path, output_path,
                                         sizeof(output_path));
        if (err != FV_OK) goto cleanup;
    }

    /* For verify-only mode, we still decrypt but don't write final output */
    if (!req->verify_only) {
        err = fv_validate_output_path(output_path, req->force_overwrite);
        if (err != FV_OK) goto cleanup;
    }

    /* Derive key from password + salt in header */
    err = fv_derive_key(req->password, password_len,
                        header.salt, header.salt_len,
                        key, FV_KEY_SIZE);
    if (err != FV_OK) goto cleanup;

    /* Calculate ciphertext size */
    int64_t file_size = fv_file_size(req->input_path);
    if (file_size < 0) {
        err = FV_ERR_FILE_READ;
        goto cleanup;
    }

    size_t header_size = fv_header_serialized_size(&header);
    int64_t ciphertext_size = file_size - (int64_t)header_size - (int64_t)header.tag_len;

    if (ciphertext_size < 0) {
        err = FV_ERR_BAD_FORMAT;
        goto cleanup;
    }

    /* Read authentication tag from end of file */
    if (fseek(fin, -(long)header.tag_len, SEEK_END) != 0) {
        err = FV_ERR_FILE_READ;
        goto cleanup;
    }

    if (fread(tag, 1, header.tag_len, fin) != header.tag_len) {
        err = FV_ERR_FILE_READ;
        goto cleanup;
    }

    /* Seek back to ciphertext start (right after header) */
    if (fseek(fin, (long)header_size, SEEK_SET) != 0) {
        err = FV_ERR_FILE_READ;
        goto cleanup;
    }

    /* Initialize AES-256-GCM decryption */
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,
                            (int)header.nonce_len, NULL) != 1) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, header.nonce) != 1) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    /* Set the expected authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG,
                            (int)header.tag_len, tag) != 1) {
        err = FV_ERR_CRYPTO_INIT;
        goto cleanup;
    }

    /* Allocate chunk buffers */
    chunk_in = (unsigned char *)malloc(FV_CHUNK_SIZE);
    chunk_out = (unsigned char *)malloc(FV_CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH);
    if (!chunk_in || !chunk_out) {
        err = FV_ERR_MEMORY;
        goto cleanup;
    }

    /* Open temp output file */
    if (!req->verify_only) {
        err = fv_generate_temp_path(output_path, temp_path, sizeof(temp_path));
        if (err != FV_OK) goto cleanup;

        fout = fopen(temp_path, "wb");
        if (!fout) {
            err = FV_ERR_FILE_OPEN;
            goto cleanup;
        }
        temp_file_created = 1;
    }

    /* Stream-decrypt ciphertext */
    int64_t remaining = ciphertext_size;
    uint64_t bytes_done = 0;
    int out_len;

    while (remaining > 0) {
        size_t to_read = (size_t)(remaining < FV_CHUNK_SIZE
                                  ? remaining : FV_CHUNK_SIZE);
        size_t bytes_read = fread(chunk_in, 1, to_read, fin);

        if (bytes_read == 0) {
            if (ferror(fin)) {
                err = FV_ERR_FILE_READ;
                goto cleanup;
            }
            break;
        }

        if (EVP_DecryptUpdate(ctx, chunk_out, &out_len,
                              chunk_in, (int)bytes_read) != 1) {
            err = FV_ERR_DECRYPT_FAILED;
            goto cleanup;
        }

        if (out_len > 0 && fout) {
            if (fwrite(chunk_out, 1, (size_t)out_len, fout) != (size_t)out_len) {
                err = FV_ERR_FILE_WRITE;
                goto cleanup;
            }
        }

        remaining -= (int64_t)bytes_read;
        bytes_done += bytes_read;

        if (req->progress_fn && ciphertext_size > 0) {
            req->progress_fn(bytes_done, (uint64_t)ciphertext_size,
                             req->progress_data);
        }
    }

    /*
     * CRITICAL: EVP_DecryptFinal_ex verifies the authentication tag.
     * If the password is wrong or the file was tampered with,
     * this call returns 0 (failure).
     */
    int final_ret = EVP_DecryptFinal_ex(ctx, chunk_out, &out_len);

    if (final_ret <= 0) {
        /* Authentication failed — wrong password or corrupted file */
        err = FV_ERR_AUTH_FAILED;
        goto cleanup;
    }

    /* Write any remaining final block */
    if (out_len > 0 && fout) {
        if (fwrite(chunk_out, 1, (size_t)out_len, fout) != (size_t)out_len) {
            err = FV_ERR_FILE_WRITE;
            goto cleanup;
        }
    }

    /* Close output before rename */
    if (fout) {
        fclose(fout);
        fout = NULL;
    }

    /* Authentication succeeded — rename temp file to final output */
    if (!req->verify_only && temp_file_created) {
        /* Remove existing output if force overwrite */
        if (req->force_overwrite && fv_file_exists(output_path)) {
            remove(output_path);
        }

        if (rename(temp_path, output_path) != 0) {
            err = FV_ERR_RENAME_FAILED;
            goto cleanup;
        }
        temp_file_created = 0; /* Don't delete in cleanup */
    }

    err = FV_OK;

    /* Fill result */
    if (result) {
        result->error = FV_OK;
        snprintf(result->input_path, sizeof(result->input_path),
                 "%s", req->input_path);
        snprintf(result->output_path, sizeof(result->output_path),
                 "%s", output_path);
        result->bytes_processed = bytes_done;
        result->duration_seconds = get_time_seconds() - start_time;
    }

cleanup:
    /* Securely wipe all sensitive material */
    fv_secure_zero(key, sizeof(key));
    fv_secure_zero(tag, sizeof(tag));

    if (chunk_in) {
        fv_secure_zero(chunk_in, FV_CHUNK_SIZE);
        free(chunk_in);
    }
    if (chunk_out) {
        fv_secure_zero(chunk_out, FV_CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH);
        free(chunk_out);
    }
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (fin) fclose(fin);
    if (fout) fclose(fout);

    /* Delete temp file on failure */
    if (temp_file_created) {
        remove(temp_path);
    }

    /* Remove partial output on failure (non-temp-file path) */
    if (err != FV_OK && result) {
        result->error = err;
    }

    return err;
}

FvError fv_file_info(const char *input_path, FvFileHeader *header) {
    if (!input_path || !header) {
        return FV_ERR_INVALID_ARGS;
    }

    FvError err = fv_validate_input_path(input_path);
    if (err != FV_OK) return err;

    FILE *fin = fopen(input_path, "rb");
    if (!fin) return FV_ERR_FILE_OPEN;

    err = fv_read_header(fin, header);
    fclose(fin);

    if (err != FV_OK) return err;

    return fv_validate_header(header);
}
