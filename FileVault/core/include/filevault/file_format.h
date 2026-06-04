/*
 * FileVault v2.0 — Secure File Encryption Tool
 * file_format.h — .vault container format read/write
 *
 * Binary layout of a .vault file:
 *
 *   Offset   Size   Field
 *   ------   ----   -----
 *   0        8      Magic: "FVAULT\0\0"
 *   8        1      Version (0x01)
 *   9        1      Algorithm ID (0x01 = AES-256-GCM)
 *   10       1      KDF ID (0x01 = PBKDF2-HMAC-SHA256)
 *   11       1      Flags (0x00 reserved)
 *   12       2      Salt length (little-endian)
 *   14       2      Nonce length (little-endian)
 *   16       2      Tag length (little-endian)
 *   18       2      Original filename length (little-endian)
 *   20       N_s    Salt bytes
 *   20+N_s   N_n    Nonce/IV bytes
 *   ...      N_f    Original filename (UTF-8, no null terminator)
 *   ...      X      Ciphertext
 *   EOF-T    T      Authentication tag
 */

#ifndef FILEVAULT_FILE_FORMAT_H
#define FILEVAULT_FILE_FORMAT_H

#include "filevault/errors.h"
#include "filevault/password.h"
#include <stdint.h>
#include <stdio.h>

/* Magic bytes identifying a FileVault encrypted file */
#define FV_MAGIC           "FVAULT\x00\x00"
#define FV_MAGIC_SIZE      8

/* Format version */
#define FV_FORMAT_VERSION  1

/* Algorithm IDs */
#define FV_ALG_AES_256_GCM          0x01

/* KDF IDs */
#define FV_KDF_PBKDF2_HMAC_SHA256   0x01

/* Maximum original filename length stored in header */
#define FV_MAX_ORIGINAL_NAME        512

/* Fixed header size (before variable-length fields) */
#define FV_FIXED_HEADER_SIZE        20

/*
 * File header structure.
 * This is the in-memory representation; the on-disk format uses
 * little-endian for multi-byte integers.
 */
typedef struct {
    uint8_t  magic[FV_MAGIC_SIZE];
    uint8_t  version;
    uint8_t  algorithm_id;
    uint8_t  kdf_id;
    uint8_t  flags;
    uint16_t salt_len;
    uint16_t nonce_len;
    uint16_t tag_len;
    uint16_t original_name_len;

    uint8_t  salt[FV_SALT_SIZE];
    uint8_t  nonce[FV_NONCE_SIZE];
    char     original_name[FV_MAX_ORIGINAL_NAME];
} FvFileHeader;

/*
 * Calculate the total serialized header size for a given header.
 */
size_t fv_header_serialized_size(const FvFileHeader *header);

/*
 * Initialize a header with default values and the given parameters.
 */
void fv_header_init(FvFileHeader *header,
                    const uint8_t *salt, uint16_t salt_len,
                    const uint8_t *nonce, uint16_t nonce_len,
                    const char *original_name);

/*
 * Write the header to a file. The file position should be at offset 0.
 * Uses little-endian encoding for multi-byte fields.
 */
FvError fv_write_header(FILE *out, const FvFileHeader *header);

/*
 * Read and validate a header from a file. The file position should be at offset 0.
 * Returns FV_ERR_BAD_FORMAT if magic bytes don't match.
 * Returns FV_ERR_UNSUPPORTED_VERSION if version is unknown.
 */
FvError fv_read_header(FILE *in, FvFileHeader *header);

/*
 * Validate an already-read header for consistency.
 */
FvError fv_validate_header(const FvFileHeader *header);

/*
 * Print header information to stdout (for the 'info' command).
 */
void fv_print_header_info(const FvFileHeader *header, const char *file_path);

/*
 * Get human-readable algorithm name.
 */
const char *fv_algorithm_name(uint8_t algorithm_id);

/*
 * Get human-readable KDF name.
 */
const char *fv_kdf_name(uint8_t kdf_id);

#endif /* FILEVAULT_FILE_FORMAT_H */
