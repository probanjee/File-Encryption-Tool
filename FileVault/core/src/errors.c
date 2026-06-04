/*
 * FileVault v2.0 — errors.c
 * Human-readable error messages, actions, and symbolic names.
 */

#include "filevault/errors.h"

typedef struct {
    FvError     code;
    const char *name;
    const char *message;
    const char *action;
} FvErrorEntry;

static const FvErrorEntry error_table[] = {
    { FV_OK,
      "FV_OK",
      "Operation completed successfully.",
      "" },

    { FV_ERR_INVALID_ARGS,
      "FV_ERR_INVALID_ARGS",
      "Invalid arguments provided.",
      "Check command syntax with 'filevault help'." },

    { FV_ERR_FILE_NOT_FOUND,
      "FV_ERR_FILE_NOT_FOUND",
      "Input file not found.",
      "Check the file path and try again." },

    { FV_ERR_FILE_OPEN,
      "FV_ERR_FILE_OPEN",
      "Failed to open file.",
      "Check file permissions and path." },

    { FV_ERR_FILE_READ,
      "FV_ERR_FILE_READ",
      "Failed to read from file.",
      "Check file permissions and disk status." },

    { FV_ERR_FILE_WRITE,
      "FV_ERR_FILE_WRITE",
      "Failed to write to file.",
      "Check disk space and write permissions." },

    { FV_ERR_FILE_EMPTY,
      "FV_ERR_FILE_EMPTY",
      "Input file is empty.",
      "Provide a non-empty file." },

    { FV_ERR_OUTPUT_EXISTS,
      "FV_ERR_OUTPUT_EXISTS",
      "Output file already exists.",
      "Use --force to overwrite, or choose a different output path with -o." },

    { FV_ERR_FILENAME_TOO_LONG,
      "FV_ERR_FILENAME_TOO_LONG",
      "Filename exceeds maximum allowed length.",
      "Use a shorter filename or specify output with -o." },

    { FV_ERR_PATH_TRAVERSAL,
      "FV_ERR_PATH_TRAVERSAL",
      "Path contains directory traversal sequences.",
      "Remove '..' from the file path." },

    { FV_ERR_EMPTY_PASSWORD,
      "FV_ERR_EMPTY_PASSWORD",
      "Password cannot be empty.",
      "Enter a password when prompted." },

    { FV_ERR_PASSWORD_MISMATCH,
      "FV_ERR_PASSWORD_MISMATCH",
      "Passwords do not match.",
      "Re-enter matching passwords." },

    { FV_ERR_PASSWORD_TOO_SHORT,
      "FV_ERR_PASSWORD_TOO_SHORT",
      "Password is too short (minimum 4 characters).",
      "Choose a longer password." },

    { FV_ERR_RANDOM_FAILED,
      "FV_ERR_RANDOM_FAILED",
      "Secure random number generation failed.",
      "Check system entropy source. On Linux, ensure /dev/urandom is available." },

    { FV_ERR_KDF_FAILED,
      "FV_ERR_KDF_FAILED",
      "Key derivation failed.",
      "This may indicate a corrupted OpenSSL installation." },

    { FV_ERR_CRYPTO_INIT,
      "FV_ERR_CRYPTO_INIT",
      "Cryptographic engine initialization failed.",
      "Check that OpenSSL is properly installed." },

    { FV_ERR_ENCRYPT_FAILED,
      "FV_ERR_ENCRYPT_FAILED",
      "Encryption operation failed.",
      "Check disk space and file permissions." },

    { FV_ERR_DECRYPT_FAILED,
      "FV_ERR_DECRYPT_FAILED",
      "Decryption operation failed.",
      "Check that the file is a valid .vault file." },

    { FV_ERR_AUTH_FAILED,
      "FV_ERR_AUTH_FAILED",
      "Authentication failed. Wrong password or corrupted file.",
      "Verify your password. If the file was modified, it cannot be recovered." },

    { FV_ERR_BAD_FORMAT,
      "FV_ERR_BAD_FORMAT",
      "File is not a valid FileVault encrypted file.",
      "Ensure you are opening a .vault file created by FileVault." },

    { FV_ERR_UNSUPPORTED_VERSION,
      "FV_ERR_UNSUPPORTED_VERSION",
      "Encrypted file version is not supported by this version of FileVault.",
      "Update FileVault to the latest version." },

    { FV_ERR_MEMORY,
      "FV_ERR_MEMORY",
      "Memory allocation failed.",
      "Close other applications and try again." },

    { FV_ERR_RENAME_FAILED,
      "FV_ERR_RENAME_FAILED",
      "Failed to rename temporary output file.",
      "Check write permissions in the output directory." },

    { FV_ERR_DELETE_FAILED,
      "FV_ERR_DELETE_FAILED",
      "Failed to delete temporary file.",
      "Manually remove any .tmp files in the output directory." },

    { FV_ERR_UNKNOWN,
      "FV_ERR_UNKNOWN",
      "An unknown error occurred.",
      "Please report this issue." },
};

static const int error_table_size =
    (int)(sizeof(error_table) / sizeof(error_table[0]));

static const FvErrorEntry *find_entry(FvError code) {
    for (int i = 0; i < error_table_size; i++) {
        if (error_table[i].code == code) {
            return &error_table[i];
        }
    }
    return &error_table[error_table_size - 1]; /* FV_ERR_UNKNOWN */
}

const char *fv_error_message(FvError code) {
    return find_entry(code)->message;
}

const char *fv_error_action(FvError code) {
    return find_entry(code)->action;
}

const char *fv_error_name(FvError code) {
    return find_entry(code)->name;
}
