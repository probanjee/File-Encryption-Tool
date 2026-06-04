/*
 * FileVault v2.0 — Secure File Encryption Tool
 * file_utils.h — File validation, path manipulation, and safe I/O
 *
 * Fixes all filename bugs from the original project:
 *   - Correct .enc stripping for legacy mode
 *   - Proper .vault output generation
 *   - Path traversal detection
 *   - Safe input reading (replaces scanf)
 */

#ifndef FILEVAULT_FILE_UTILS_H
#define FILEVAULT_FILE_UTILS_H

#include "filevault/errors.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Maximum path length used throughout the project */
#define FV_MAX_PATH 1024

/*
 * Check if a file exists and is readable.
 * Returns 1 if exists, 0 if not.
 */
int fv_file_exists(const char *path);

/*
 * Get file size in bytes. Returns -1 on error.
 */
int64_t fv_file_size(const char *path);

/*
 * Validate an input file path:
 *   - Not NULL or empty
 *   - No path traversal (..)
 *   - File exists
 *   - File is not empty
 *   - Filename not too long
 */
FvError fv_validate_input_path(const char *path);

/*
 * Validate an output file path:
 *   - Not NULL or empty
 *   - No path traversal
 *   - File does not already exist (unless force is set)
 *   - Filename not too long
 */
FvError fv_validate_output_path(const char *path, int force);

/*
 * Strip a file extension if present.
 * Example: fv_strip_extension("King.txt.enc", ".enc", out, 1024)
 *   → out = "King.txt"
 * If the extension is not found, copies input unchanged.
 * Returns FV_OK or FV_ERR_FILENAME_TOO_LONG.
 */
FvError fv_strip_extension(const char *input, const char *ext,
                           char *output, size_t output_size);

/*
 * Generate encrypted output filename: input + ".vault"
 * Example: "King.txt" → "King.txt.vault"
 */
FvError fv_generate_encrypt_output(const char *input,
                                   char *output, size_t output_size);

/*
 * Generate decrypted output filename by stripping ".vault" and appending ".dec"
 * Example: "King.txt.vault" → "King.txt.dec"
 * If no ".vault" extension found, appends ".dec" directly.
 */
FvError fv_generate_decrypt_output(const char *input,
                                   char *output, size_t output_size);

/*
 * Generate legacy XOR encrypted output: input + ".enc"
 */
FvError fv_generate_legacy_encrypt_output(const char *input,
                                          char *output, size_t output_size);

/*
 * Generate legacy XOR decrypted output by stripping ".enc" and appending ".dec"
 * Example: "King.txt.enc" → "King.txt.dec"  (NOT "King.txt.enc.dec")
 */
FvError fv_generate_legacy_decrypt_output(const char *input,
                                          char *output, size_t output_size);

/*
 * Generate legacy key file path by stripping ".enc" and appending ".key"
 * Example: "King.txt.enc" → "King.txt.key"  (NOT "King.txt.enc.key")
 */
FvError fv_generate_legacy_key_path(const char *encrypted_path,
                                    char *key_path, size_t key_path_size);

/*
 * Check if a path contains directory traversal sequences ("..").
 * Returns 1 if traversal detected, 0 if safe.
 */
int fv_has_path_traversal(const char *path);

/*
 * Safe line reading — replaces scanf("%s").
 * Reads up to buf_size-1 characters from stream, strips trailing newline.
 * Returns FV_OK on success, FV_ERR_FILE_READ on failure.
 */
FvError fv_safe_read_line(char *buf, size_t buf_size, FILE *stream);

/*
 * Extract just the filename from a full path.
 * Example: "/home/user/docs/King.txt" → "King.txt"
 */
const char *fv_basename(const char *path);

/*
 * Generate a temporary file path for safe decryption.
 * Example: "output.dec" → "output.dec.tmp"
 */
FvError fv_generate_temp_path(const char *output_path,
                              char *temp_path, size_t temp_size);

#endif /* FILEVAULT_FILE_UTILS_H */
