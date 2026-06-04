/*
 * FileVault v2.0 — password.c
 * Password input (echo-suppressed) and PBKDF2-HMAC-SHA256 key derivation.
 */

#include "filevault/password.h"
#include "filevault/secure_mem.h"
#include <stdio.h>
#include <string.h>

/* OpenSSL headers */
#include <openssl/evp.h>
#include <openssl/rand.h>

/* Platform-specific terminal echo control */
#ifdef _WIN32
#  include <windows.h>
#  include <conio.h>
#else
#  include <termios.h>
#  include <unistd.h>
#endif

FvError fv_generate_random(unsigned char *buf, size_t len) {
    if (!buf || len == 0) return FV_ERR_INVALID_ARGS;

    if (RAND_bytes(buf, (int)len) != 1) {
        return FV_ERR_RANDOM_FAILED;
    }
    return FV_OK;
}

FvError fv_derive_key(const char *password, size_t password_len,
                      const unsigned char *salt, size_t salt_len,
                      unsigned char *out_key, size_t key_len) {
    if (!password || password_len == 0) return FV_ERR_EMPTY_PASSWORD;
    if (!salt || salt_len == 0) return FV_ERR_INVALID_ARGS;
    if (!out_key || key_len == 0) return FV_ERR_INVALID_ARGS;

    int result = PKCS5_PBKDF2_HMAC(
        password, (int)password_len,
        salt, (int)salt_len,
        FV_PBKDF2_ITERS,
        EVP_sha256(),
        (int)key_len,
        out_key
    );

    if (result != 1) {
        fv_secure_zero(out_key, key_len);
        return FV_ERR_KDF_FAILED;
    }

    return FV_OK;
}

FvError fv_validate_password(const char *password, size_t len) {
    if (!password || len == 0) {
        return FV_ERR_EMPTY_PASSWORD;
    }
    if (len < FV_MIN_PASSWORD) {
        return FV_ERR_PASSWORD_TOO_SHORT;
    }
    return FV_OK;
}

#ifdef _WIN32
/* Windows: use SetConsoleMode to disable echo */
static FvError read_password_platform(char *buf, size_t buf_size) {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);

    size_t pos = 0;
    int ch;

    while (pos < buf_size - 1) {
        ch = _getch();

        if (ch == '\r' || ch == '\n') {
            break;
        } else if (ch == '\b' || ch == 127) {
            /* Backspace */
            if (pos > 0) {
                pos--;
                fprintf(stderr, "\b \b");
            }
        } else if (ch >= 32 && ch <= 126) {
            buf[pos++] = (char)ch;
            fprintf(stderr, "*");
        }
    }
    buf[pos] = '\0';

    SetConsoleMode(hStdin, mode);
    fprintf(stderr, "\n");

    return FV_OK;
}
#else
/* POSIX: use termios to disable echo */
static FvError read_password_platform(char *buf, size_t buf_size) {
    struct termios old_term, new_term;

    /* Get current terminal settings */
    if (tcgetattr(STDIN_FILENO, &old_term) != 0) {
        /* Fallback: terminal might not be available (piped input) */
        if (!fgets(buf, (int)buf_size, stdin)) {
            return FV_ERR_FILE_READ;
        }
        /* Strip newline */
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[--len] = '\0';
        return FV_OK;
    }

    /* Disable echo */
    new_term = old_term;
    new_term.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    /* Read input character by character for visual feedback */
    size_t pos = 0;
    int ch;

    while (pos < buf_size - 1) {
        ch = getchar();
        if (ch == '\n' || ch == EOF) {
            break;
        } else if (ch == 127 || ch == '\b') {
            /* Backspace */
            if (pos > 0) {
                pos--;
                fprintf(stderr, "\b \b");
            }
        } else if (ch >= 32 && ch <= 126) {
            buf[pos++] = (char)ch;
            fprintf(stderr, "*");
        }
    }
    buf[pos] = '\0';

    /* Restore terminal */
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    fprintf(stderr, "\n");

    return FV_OK;
}
#endif

FvError fv_read_password(char *buf, size_t buf_size, const char *prompt) {
    if (!buf || buf_size < 2) return FV_ERR_INVALID_ARGS;

    memset(buf, 0, buf_size);

    if (prompt) {
        fprintf(stderr, "%s", prompt);
        fflush(stderr);
    }

    FvError err = read_password_platform(buf, buf_size);
    if (err != FV_OK) return err;

    size_t len = strlen(buf);
    if (len == 0) {
        return FV_ERR_EMPTY_PASSWORD;
    }

    return FV_OK;
}

FvError fv_read_password_confirmed(char *buf, size_t buf_size) {
    char confirm[FV_MAX_PASSWORD];
    memset(confirm, 0, sizeof(confirm));

    /* First password entry */
    FvError err = fv_read_password(buf, buf_size, "  Enter password: ");
    if (err != FV_OK) return err;

    /* Validate minimum length */
    err = fv_validate_password(buf, strlen(buf));
    if (err != FV_OK) {
        fv_secure_zero(buf, buf_size);
        return err;
    }

    /* Confirmation */
    err = fv_read_password(confirm, sizeof(confirm), "  Confirm password: ");
    if (err != FV_OK) {
        fv_secure_zero(buf, buf_size);
        fv_secure_zero(confirm, sizeof(confirm));
        return err;
    }

    /* Compare */
    if (strcmp(buf, confirm) != 0) {
        fv_secure_zero(buf, buf_size);
        fv_secure_zero(confirm, sizeof(confirm));
        return FV_ERR_PASSWORD_MISMATCH;
    }

    fv_secure_zero(confirm, sizeof(confirm));
    return FV_OK;
}
