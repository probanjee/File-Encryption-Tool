/*
 * FileVault v2.0 — Secure File Encryption Tool
 * password.h — Password handling and key derivation
 *
 * Uses PBKDF2-HMAC-SHA256 via OpenSSL for key derivation.
 * Provides secure password input with echo suppression.
 */

#ifndef FILEVAULT_PASSWORD_H
#define FILEVAULT_PASSWORD_H

#include "filevault/errors.h"
#include <stddef.h>
#include <stdint.h>

/* Cryptographic parameters */
#define FV_SALT_SIZE      16       /* 128-bit random salt */
#define FV_KEY_SIZE       32       /* 256-bit derived key */
#define FV_NONCE_SIZE     12       /* 96-bit nonce for AES-256-GCM */
#define FV_TAG_SIZE       16       /* 128-bit authentication tag */
#define FV_PBKDF2_ITERS   600000   /* OWASP 2024 recommendation for HMAC-SHA256 */
#define FV_MAX_PASSWORD   1024     /* Maximum password length */
#define FV_MIN_PASSWORD   4        /* Minimum password length */

/*
 * Generate cryptographically secure random bytes.
 * Uses OpenSSL RAND_bytes internally.
 */
FvError fv_generate_random(unsigned char *buf, size_t len);

/*
 * Derive a 256-bit encryption key from a password and salt
 * using PBKDF2-HMAC-SHA256.
 *
 * password      - User-supplied password (UTF-8)
 * password_len  - Length of password in bytes (not including null terminator)
 * salt          - Random salt (should be FV_SALT_SIZE bytes)
 * salt_len      - Length of salt
 * out_key       - Output buffer for derived key (must be FV_KEY_SIZE bytes)
 * key_len       - Size of out_key buffer
 */
FvError fv_derive_key(const char *password, size_t password_len,
                      const unsigned char *salt, size_t salt_len,
                      unsigned char *out_key, size_t key_len);

/*
 * Read a password from the terminal with echo suppressed.
 * Displays the given prompt string.
 * The password is stored in buf (null-terminated).
 * Returns FV_ERR_EMPTY_PASSWORD if the user enters nothing.
 */
FvError fv_read_password(char *buf, size_t buf_size, const char *prompt);

/*
 * Read a password with confirmation (asks twice, rejects mismatch).
 * Used during encryption.
 */
FvError fv_read_password_confirmed(char *buf, size_t buf_size);

/*
 * Validate password meets minimum requirements.
 */
FvError fv_validate_password(const char *password, size_t len);

#endif /* FILEVAULT_PASSWORD_H */
