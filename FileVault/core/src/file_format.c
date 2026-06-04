/*
 * FileVault v2.0 — file_format.c
 * .vault container format serialization and deserialization.
 * All multi-byte integers are stored in little-endian byte order.
 */

#include "filevault/file_format.h"
#include "filevault/file_utils.h"
#include "filevault/version.h"
#include <string.h>
#include <stdio.h>

/* Little-endian helpers */
static void write_le16(uint8_t *buf, uint16_t val) {
    buf[0] = (uint8_t)(val & 0xFF);
    buf[1] = (uint8_t)((val >> 8) & 0xFF);
}

static uint16_t read_le16(const uint8_t *buf) {
    return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

const char *fv_algorithm_name(uint8_t algorithm_id) {
    switch (algorithm_id) {
        case FV_ALG_AES_256_GCM: return "AES-256-GCM";
        default:                 return "Unknown";
    }
}

const char *fv_kdf_name(uint8_t kdf_id) {
    switch (kdf_id) {
        case FV_KDF_PBKDF2_HMAC_SHA256: return "PBKDF2-HMAC-SHA256";
        default:                         return "Unknown";
    }
}

size_t fv_header_serialized_size(const FvFileHeader *header) {
    if (!header) return 0;
    return FV_FIXED_HEADER_SIZE
         + header->salt_len
         + header->nonce_len
         + header->original_name_len;
}

void fv_header_init(FvFileHeader *header,
                    const uint8_t *salt, uint16_t salt_len,
                    const uint8_t *nonce, uint16_t nonce_len,
                    const char *original_name) {
    if (!header) return;

    memset(header, 0, sizeof(FvFileHeader));

    /* Magic bytes */
    memcpy(header->magic, FV_MAGIC, FV_MAGIC_SIZE);

    /* Version and algorithm */
    header->version      = FV_FORMAT_VERSION;
    header->algorithm_id = FV_ALG_AES_256_GCM;
    header->kdf_id       = FV_KDF_PBKDF2_HMAC_SHA256;
    header->flags        = 0;

    /* Lengths */
    header->salt_len  = salt_len;
    header->nonce_len = nonce_len;
    header->tag_len   = FV_TAG_SIZE;

    /* Salt */
    if (salt && salt_len > 0 && salt_len <= FV_SALT_SIZE) {
        memcpy(header->salt, salt, salt_len);
    }

    /* Nonce */
    if (nonce && nonce_len > 0 && nonce_len <= FV_NONCE_SIZE) {
        memcpy(header->nonce, nonce, nonce_len);
    }

    /* Original filename */
    if (original_name) {
        const char *basename = fv_basename(original_name);
        size_t name_len = strlen(basename);
        if (name_len > FV_MAX_ORIGINAL_NAME - 1) {
            name_len = FV_MAX_ORIGINAL_NAME - 1;
        }
        header->original_name_len = (uint16_t)name_len;
        memcpy(header->original_name, basename, name_len);
    }
}

FvError fv_write_header(FILE *out, const FvFileHeader *header) {
    if (!out || !header) return FV_ERR_INVALID_ARGS;

    uint8_t fixed[FV_FIXED_HEADER_SIZE];

    /* Pack fixed header */
    memcpy(fixed, header->magic, FV_MAGIC_SIZE);   /* 0-7 */
    fixed[8]  = header->version;                    /* 8 */
    fixed[9]  = header->algorithm_id;               /* 9 */
    fixed[10] = header->kdf_id;                     /* 10 */
    fixed[11] = header->flags;                      /* 11 */
    write_le16(&fixed[12], header->salt_len);       /* 12-13 */
    write_le16(&fixed[14], header->nonce_len);      /* 14-15 */
    write_le16(&fixed[16], header->tag_len);        /* 16-17 */
    write_le16(&fixed[18], header->original_name_len); /* 18-19 */

    /* Write fixed header */
    if (fwrite(fixed, 1, FV_FIXED_HEADER_SIZE, out) != FV_FIXED_HEADER_SIZE) {
        return FV_ERR_FILE_WRITE;
    }

    /* Write salt */
    if (header->salt_len > 0) {
        if (fwrite(header->salt, 1, header->salt_len, out) != header->salt_len) {
            return FV_ERR_FILE_WRITE;
        }
    }

    /* Write nonce */
    if (header->nonce_len > 0) {
        if (fwrite(header->nonce, 1, header->nonce_len, out) != header->nonce_len) {
            return FV_ERR_FILE_WRITE;
        }
    }

    /* Write original filename */
    if (header->original_name_len > 0) {
        if (fwrite(header->original_name, 1, header->original_name_len, out)
            != header->original_name_len) {
            return FV_ERR_FILE_WRITE;
        }
    }

    return FV_OK;
}

FvError fv_read_header(FILE *in, FvFileHeader *header) {
    if (!in || !header) return FV_ERR_INVALID_ARGS;

    memset(header, 0, sizeof(FvFileHeader));

    /* Read fixed header */
    uint8_t fixed[FV_FIXED_HEADER_SIZE];
    if (fread(fixed, 1, FV_FIXED_HEADER_SIZE, in) != FV_FIXED_HEADER_SIZE) {
        return FV_ERR_BAD_FORMAT;
    }

    /* Check magic bytes */
    if (memcmp(fixed, FV_MAGIC, FV_MAGIC_SIZE) != 0) {
        return FV_ERR_BAD_FORMAT;
    }

    /* Unpack fixed fields */
    memcpy(header->magic, fixed, FV_MAGIC_SIZE);
    header->version           = fixed[8];
    header->algorithm_id      = fixed[9];
    header->kdf_id            = fixed[10];
    header->flags             = fixed[11];
    header->salt_len          = read_le16(&fixed[12]);
    header->nonce_len         = read_le16(&fixed[14]);
    header->tag_len           = read_le16(&fixed[16]);
    header->original_name_len = read_le16(&fixed[18]);

    /* Validate version */
    if (header->version != FV_FORMAT_VERSION) {
        return FV_ERR_UNSUPPORTED_VERSION;
    }

    /* Validate lengths */
    if (header->salt_len > FV_SALT_SIZE ||
        header->nonce_len > FV_NONCE_SIZE ||
        header->tag_len > 32 ||
        header->original_name_len >= FV_MAX_ORIGINAL_NAME) {
        return FV_ERR_BAD_FORMAT;
    }

    /* Read salt */
    if (header->salt_len > 0) {
        if (fread(header->salt, 1, header->salt_len, in) != header->salt_len) {
            return FV_ERR_FILE_READ;
        }
    }

    /* Read nonce */
    if (header->nonce_len > 0) {
        if (fread(header->nonce, 1, header->nonce_len, in) != header->nonce_len) {
            return FV_ERR_FILE_READ;
        }
    }

    /* Read original filename */
    if (header->original_name_len > 0) {
        if (fread(header->original_name, 1, header->original_name_len, in)
            != header->original_name_len) {
            return FV_ERR_FILE_READ;
        }
        header->original_name[header->original_name_len] = '\0';
    }

    return FV_OK;
}

FvError fv_validate_header(const FvFileHeader *header) {
    if (!header) return FV_ERR_INVALID_ARGS;

    if (memcmp(header->magic, FV_MAGIC, FV_MAGIC_SIZE) != 0) {
        return FV_ERR_BAD_FORMAT;
    }
    if (header->version != FV_FORMAT_VERSION) {
        return FV_ERR_UNSUPPORTED_VERSION;
    }
    if (header->algorithm_id != FV_ALG_AES_256_GCM) {
        return FV_ERR_BAD_FORMAT;
    }
    if (header->kdf_id != FV_KDF_PBKDF2_HMAC_SHA256) {
        return FV_ERR_BAD_FORMAT;
    }
    if (header->salt_len == 0 || header->salt_len > FV_SALT_SIZE) {
        return FV_ERR_BAD_FORMAT;
    }
    if (header->nonce_len == 0 || header->nonce_len > FV_NONCE_SIZE) {
        return FV_ERR_BAD_FORMAT;
    }
    if (header->tag_len == 0 || header->tag_len > 32) {
        return FV_ERR_BAD_FORMAT;
    }

    return FV_OK;
}

void fv_print_header_info(const FvFileHeader *header, const char *file_path) {
    if (!header) return;

    printf("\n");
    printf("  Encrypted File Info\n");
    printf("  ────────────────────────────────────────\n");

    if (file_path) {
        printf("  File          : %s\n", file_path);
    }

    printf("  Format        : FileVault Encrypted File\n");
    printf("  Version       : %u\n", header->version);
    printf("  Algorithm     : %s\n", fv_algorithm_name(header->algorithm_id));
    printf("  KDF           : %s\n", fv_kdf_name(header->kdf_id));
    printf("  Salt size     : %u bytes\n", header->salt_len);
    printf("  Nonce size    : %u bytes\n", header->nonce_len);
    printf("  Tag size      : %u bytes\n", header->tag_len);

    if (header->original_name_len > 0) {
        printf("  Original name : %.*s\n",
               (int)header->original_name_len, header->original_name);
    } else {
        printf("  Original name : (not stored)\n");
    }

    printf("  Integrity     : Authenticated encryption (GCM)\n");
    printf("  ────────────────────────────────────────\n");
    printf("\n");
}
