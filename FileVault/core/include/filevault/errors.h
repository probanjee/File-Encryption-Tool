/*
 * FileVault v2.0 — Secure File Encryption Tool
 * errors.h — Structured error codes and messages
 *
 * Every public function in the core library returns FvError.
 * Use fv_error_message() and fv_error_action() to get human-readable strings.
 */

#ifndef FILEVAULT_ERRORS_H
#define FILEVAULT_ERRORS_H

typedef enum {
    FV_OK = 0,

    /* Argument / input errors */
    FV_ERR_INVALID_ARGS       = 1,
    FV_ERR_FILE_NOT_FOUND     = 2,
    FV_ERR_FILE_OPEN          = 3,
    FV_ERR_FILE_READ          = 4,
    FV_ERR_FILE_WRITE         = 5,
    FV_ERR_FILE_EMPTY         = 6,
    FV_ERR_OUTPUT_EXISTS      = 7,
    FV_ERR_FILENAME_TOO_LONG  = 8,
    FV_ERR_PATH_TRAVERSAL     = 9,

    /* Password / key errors */
    FV_ERR_EMPTY_PASSWORD     = 10,
    FV_ERR_PASSWORD_MISMATCH  = 11,
    FV_ERR_PASSWORD_TOO_SHORT = 12,

    /* Crypto errors */
    FV_ERR_RANDOM_FAILED      = 20,
    FV_ERR_KDF_FAILED         = 21,
    FV_ERR_CRYPTO_INIT        = 22,
    FV_ERR_ENCRYPT_FAILED     = 23,
    FV_ERR_DECRYPT_FAILED     = 24,
    FV_ERR_AUTH_FAILED         = 25,

    /* Format errors */
    FV_ERR_BAD_FORMAT         = 30,
    FV_ERR_UNSUPPORTED_VERSION = 31,

    /* System errors */
    FV_ERR_MEMORY             = 40,
    FV_ERR_RENAME_FAILED      = 41,
    FV_ERR_DELETE_FAILED      = 42,

    FV_ERR_UNKNOWN            = 99
} FvError;

/*
 * Returns a human-readable error message for the given error code.
 * Never returns NULL — returns "Unknown error" for unrecognized codes.
 */
const char *fv_error_message(FvError code);

/*
 * Returns a suggested corrective action for the given error code.
 * Never returns NULL — returns empty string if no specific action is suggested.
 */
const char *fv_error_action(FvError code);

/*
 * Returns the symbolic name of the error code (e.g., "FV_ERR_AUTH_FAILED").
 * Useful for logging and debugging.
 */
const char *fv_error_name(FvError code);

#endif /* FILEVAULT_ERRORS_H */
