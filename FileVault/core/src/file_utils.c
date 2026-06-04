/*
 * FileVault v2.0 — file_utils.c
 * File validation, path manipulation, and safe I/O.
 *
 * This module fixes the critical filename bugs from the original project:
 *   BUG 1: Decryption of "King.txt.enc" searched for "King.txt.enc.key"
 *          → Fixed: strips ".enc" first → finds "King.txt.key"
 *   BUG 2: Decrypted output was "King.txt.enc.dec"
 *          → Fixed: strips ".enc" first → produces "King.txt.dec"
 */

#include "filevault/file_utils.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#  include <io.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  define stat_t struct _stat64
#  define fstat_fn _stat64
#else
#  include <sys/stat.h>
#  include <unistd.h>
#  define stat_t struct stat
#  define fstat_fn stat
#endif

int fv_file_exists(const char *path) {
    if (!path) return 0;
    stat_t st;
    return (fstat_fn(path, &st) == 0);
}

int64_t fv_file_size(const char *path) {
    if (!path) return -1;
    stat_t st;
    if (fstat_fn(path, &st) != 0) {
        return -1;
    }
    return (int64_t)st.st_size;
}

int fv_has_path_traversal(const char *path) {
    if (!path) return 0;

    const char *p = path;
    while (*p) {
        /* Check for ".." preceded by start-of-string or separator */
        if (p[0] == '.' && p[1] == '.') {
            /* Check character before: must be start, '/', or '\\' */
            if (p == path || p[-1] == '/' || p[-1] == '\\') {
                /* Check character after: must be end, '/', or '\\' */
                if (p[2] == '\0' || p[2] == '/' || p[2] == '\\') {
                    return 1;
                }
            }
        }
        p++;
    }
    return 0;
}

FvError fv_validate_input_path(const char *path) {
    if (!path || path[0] == '\0') {
        return FV_ERR_INVALID_ARGS;
    }
    if (strlen(path) >= FV_MAX_PATH) {
        return FV_ERR_FILENAME_TOO_LONG;
    }
    if (fv_has_path_traversal(path)) {
        return FV_ERR_PATH_TRAVERSAL;
    }
    if (!fv_file_exists(path)) {
        return FV_ERR_FILE_NOT_FOUND;
    }
    int64_t size = fv_file_size(path);
    if (size == 0) {
        return FV_ERR_FILE_EMPTY;
    }
    if (size < 0) {
        return FV_ERR_FILE_READ;
    }
    return FV_OK;
}

FvError fv_validate_output_path(const char *path, int force) {
    if (!path || path[0] == '\0') {
        return FV_ERR_INVALID_ARGS;
    }
    if (strlen(path) >= FV_MAX_PATH) {
        return FV_ERR_FILENAME_TOO_LONG;
    }
    if (fv_has_path_traversal(path)) {
        return FV_ERR_PATH_TRAVERSAL;
    }
    if (!force && fv_file_exists(path)) {
        return FV_ERR_OUTPUT_EXISTS;
    }
    return FV_OK;
}

FvError fv_strip_extension(const char *input, const char *ext,
                           char *output, size_t output_size) {
    if (!input || !ext || !output || output_size == 0) {
        return FV_ERR_INVALID_ARGS;
    }

    size_t input_len = strlen(input);
    size_t ext_len   = strlen(ext);

    /* Check if input ends with the extension */
    if (input_len > ext_len &&
        strcmp(input + input_len - ext_len, ext) == 0) {
        /* Strip the extension */
        size_t base_len = input_len - ext_len;
        if (base_len >= output_size) {
            return FV_ERR_FILENAME_TOO_LONG;
        }
        memcpy(output, input, base_len);
        output[base_len] = '\0';
    } else {
        /* No matching extension — copy unchanged */
        if (input_len >= output_size) {
            return FV_ERR_FILENAME_TOO_LONG;
        }
        memcpy(output, input, input_len + 1);
    }
    return FV_OK;
}

FvError fv_generate_encrypt_output(const char *input,
                                   char *output, size_t output_size) {
    if (!input || !output) return FV_ERR_INVALID_ARGS;
    size_t needed = strlen(input) + 7; /* ".vault" + null */
    if (needed > output_size) return FV_ERR_FILENAME_TOO_LONG;
    snprintf(output, output_size, "%s.vault", input);
    return FV_OK;
}

FvError fv_generate_decrypt_output(const char *input,
                                   char *output, size_t output_size) {
    if (!input || !output) return FV_ERR_INVALID_ARGS;

    /* Strip ".vault" extension first, then append ".dec" */
    char base[FV_MAX_PATH];
    FvError err = fv_strip_extension(input, ".vault", base, sizeof(base));
    if (err != FV_OK) return err;

    size_t needed = strlen(base) + 5; /* ".dec" + null */
    if (needed > output_size) return FV_ERR_FILENAME_TOO_LONG;
    snprintf(output, output_size, "%s.dec", base);
    return FV_OK;
}

FvError fv_generate_legacy_encrypt_output(const char *input,
                                          char *output, size_t output_size) {
    if (!input || !output) return FV_ERR_INVALID_ARGS;
    size_t needed = strlen(input) + 5; /* ".enc" + null */
    if (needed > output_size) return FV_ERR_FILENAME_TOO_LONG;
    snprintf(output, output_size, "%s.enc", input);
    return FV_OK;
}

FvError fv_generate_legacy_decrypt_output(const char *input,
                                          char *output, size_t output_size) {
    if (!input || !output) return FV_ERR_INVALID_ARGS;

    /*
     * BUG FIX: Original code produced "King.txt.enc.dec"
     * Correct behavior: strip ".enc" → "King.txt" → append ".dec" → "King.txt.dec"
     */
    char base[FV_MAX_PATH];
    FvError err = fv_strip_extension(input, ".enc", base, sizeof(base));
    if (err != FV_OK) return err;

    size_t needed = strlen(base) + 5;
    if (needed > output_size) return FV_ERR_FILENAME_TOO_LONG;
    snprintf(output, output_size, "%s.dec", base);
    return FV_OK;
}

FvError fv_generate_legacy_key_path(const char *encrypted_path,
                                    char *key_path, size_t key_path_size) {
    if (!encrypted_path || !key_path) return FV_ERR_INVALID_ARGS;

    /*
     * BUG FIX: Original code looked for "King.txt.enc.key"
     * Correct behavior: strip ".enc" → "King.txt" → append ".key" → "King.txt.key"
     */
    char base[FV_MAX_PATH];
    FvError err = fv_strip_extension(encrypted_path, ".enc", base, sizeof(base));
    if (err != FV_OK) return err;

    size_t needed = strlen(base) + 5;
    if (needed > key_path_size) return FV_ERR_FILENAME_TOO_LONG;
    snprintf(key_path, key_path_size, "%s.key", base);
    return FV_OK;
}

const char *fv_basename(const char *path) {
    if (!path) return "";

    const char *last_sep = path;
    const char *p = path;
    while (*p) {
        if (*p == '/' || *p == '\\') {
            last_sep = p + 1;
        }
        p++;
    }
    /* If no separator found, last_sep == path (the original pointer) */
    if (last_sep == path && *path != '/' && *path != '\\') {
        return path;
    }
    return last_sep;
}

FvError fv_safe_read_line(char *buf, size_t buf_size, FILE *stream) {
    if (!buf || buf_size == 0 || !stream) {
        return FV_ERR_INVALID_ARGS;
    }

    if (!fgets(buf, (int)buf_size, stream)) {
        buf[0] = '\0';
        return FV_ERR_FILE_READ;
    }

    /* Strip trailing newline and carriage return */
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[--len] = '\0';
    }

    return FV_OK;
}

FvError fv_generate_temp_path(const char *output_path,
                              char *temp_path, size_t temp_size) {
    if (!output_path || !temp_path) return FV_ERR_INVALID_ARGS;
    size_t needed = strlen(output_path) + 5; /* ".tmp" + null */
    if (needed > temp_size) return FV_ERR_FILENAME_TOO_LONG;
    snprintf(temp_path, temp_size, "%s.tmp", output_path);
    return FV_OK;
}
