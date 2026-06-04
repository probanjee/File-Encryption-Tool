/*
 * FileVault v2.0 — cli_commands.c
 * Command handlers that bridge CLI arguments → core library calls.
 */

#include "cli_commands.h"
#include "cli_progress.h"

#include "filevault/version.h"
#include "filevault/errors.h"
#include "filevault/crypto_engine.h"
#include "filevault/legacy_xor.h"
#include "filevault/password.h"
#include "filevault/file_utils.h"
#include "filevault/secure_mem.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_banner(void) {
    printf("\n");
    printf("  ╔══════════════════════════════════════════╗\n");
    printf("  ║       FileVault v%s                    ║\n", FV_VERSION_STRING);
    printf("  ║       Secure File Encryption             ║\n");
    printf("  ╚══════════════════════════════════════════╝\n");
    printf("\n");
}

static void print_error(FvError err) {
    fprintf(stderr, "\n  [ERROR] %s\n", fv_error_message(err));
    const char *action = fv_error_action(err);
    if (action && action[0] != '\0') {
        fprintf(stderr, "  [HINT]  %s\n", action);
    }
    fprintf(stderr, "\n");
}

static void print_result(const char *mode, const FvResult *result) {
    char size_buf[32];
    cli_format_size(result->bytes_processed, size_buf, sizeof(size_buf));

    printf("\n");
    printf("  [OK] %s completed successfully.\n", mode);
    printf("  Output   : %s\n", result->output_path);
    printf("  Size     : %s\n", size_buf);
    printf("  Duration : %.2fs\n", result->duration_seconds);
    printf("\n");
}

int cmd_encrypt(const char *input_path, const char *output_path,
                int force_overwrite) {
    print_banner();
    printf("  Mode      : Encrypt\n");
    printf("  Input     : %s\n", input_path);
    printf("  Algorithm : AES-256-GCM\n");
    printf("  KDF       : PBKDF2-HMAC-SHA256 (%d iterations)\n", FV_PBKDF2_ITERS);
    printf("\n");

    /* Read and confirm password */
    char password[FV_MAX_PASSWORD];
    memset(password, 0, sizeof(password));

    FvError err = fv_read_password_confirmed(password, sizeof(password));
    if (err != FV_OK) {
        fv_secure_zero(password, sizeof(password));
        print_error(err);
        return 1;
    }

    printf("\n");

    /* Set up progress tracking */
    int64_t fsize = fv_file_size(input_path);
    CliProgress prog;
    cli_progress_init(&prog, fsize > 0 ? (uint64_t)fsize : 0);

    /* Build request */
    FvEncryptRequest req = {
        .input_path      = input_path,
        .output_path     = output_path,
        .password        = password,
        .force_overwrite = force_overwrite,
        .progress_fn     = cli_progress_update,
        .progress_data   = &prog
    };

    FvResult result;
    err = fv_encrypt_file(&req, &result);

    cli_progress_finish(&prog);

    /* Wipe password immediately */
    fv_secure_zero(password, sizeof(password));

    if (err != FV_OK) {
        print_error(err);
        return 1;
    }

    print_result("Encryption", &result);
    return 0;
}

int cmd_decrypt(const char *input_path, const char *output_path,
                int force_overwrite) {
    print_banner();
    printf("  Mode      : Decrypt\n");
    printf("  Input     : %s\n", input_path);
    printf("\n");

    /* Read password (single entry, no confirmation) */
    char password[FV_MAX_PASSWORD];
    memset(password, 0, sizeof(password));

    FvError err = fv_read_password(password, sizeof(password),
                                   "  Enter password: ");
    if (err != FV_OK) {
        fv_secure_zero(password, sizeof(password));
        print_error(err);
        return 1;
    }

    printf("\n");

    /* Set up progress tracking */
    int64_t fsize = fv_file_size(input_path);
    CliProgress prog;
    cli_progress_init(&prog, fsize > 0 ? (uint64_t)fsize : 0);

    /* Build request */
    FvDecryptRequest req = {
        .input_path      = input_path,
        .output_path     = output_path,
        .password        = password,
        .force_overwrite = force_overwrite,
        .verify_only     = 0,
        .progress_fn     = cli_progress_update,
        .progress_data   = &prog
    };

    FvResult result;
    err = fv_decrypt_file(&req, &result);

    cli_progress_finish(&prog);

    /* Wipe password immediately */
    fv_secure_zero(password, sizeof(password));

    if (err != FV_OK) {
        print_error(err);
        fprintf(stderr, "  [INFO] No decrypted output was created.\n\n");
        return 1;
    }

    print_result("Decryption", &result);
    return 0;
}

int cmd_info(const char *input_path) {
    print_banner();

    FvFileHeader header;
    FvError err = fv_file_info(input_path, &header);

    if (err != FV_OK) {
        print_error(err);
        return 1;
    }

    fv_print_header_info(&header, input_path);
    return 0;
}

int cmd_legacy_xor_encrypt(const char *input_path) {
    print_banner();
    printf("  Mode      : Legacy XOR Encrypt\n");
    printf("  Input     : %s\n", input_path);
    printf("\n");

    /* Read key from user */
    char key[FV_LEGACY_MAX_KEY];
    memset(key, 0, sizeof(key));

    FvError err = fv_read_password(key, sizeof(key),
                                   "  Enter encryption key: ");
    if (err != FV_OK) {
        fv_secure_zero(key, sizeof(key));
        print_error(err);
        return 1;
    }

    printf("\n");

    CliProgress prog;
    int64_t fsize = fv_file_size(input_path);
    cli_progress_init(&prog, fsize > 0 ? (uint64_t)fsize : 0);

    err = fv_legacy_xor_encrypt_auto(input_path, key, strlen(key),
                                     cli_progress_update, &prog);

    cli_progress_finish(&prog);
    fv_secure_zero(key, sizeof(key));

    if (err != FV_OK) {
        print_error(err);
        return 1;
    }

    printf("\n  [OK] Legacy XOR encryption completed.\n\n");
    return 0;
}

int cmd_legacy_xor_decrypt(const char *input_path,
                           const char *key_file_path) {
    print_banner();
    printf("  Mode      : Legacy XOR Decrypt\n");
    printf("  Input     : %s\n", input_path);
    printf("\n");

    CliProgress prog;
    int64_t fsize = fv_file_size(input_path);
    cli_progress_init(&prog, fsize > 0 ? (uint64_t)fsize : 0);

    FvError err = fv_legacy_xor_decrypt(input_path, key_file_path, NULL,
                                        cli_progress_update, &prog);

    cli_progress_finish(&prog);

    if (err != FV_OK) {
        print_error(err);
        return 1;
    }

    printf("\n  [OK] Legacy XOR decryption completed.\n\n");
    return 0;
}

void cmd_help(void) {
    print_banner();

    printf("  Usage:\n");
    printf("    filevault encrypt <input> [-o <output>] [--force]\n");
    printf("    filevault decrypt <input.vault> [-o <output>] [--force]\n");
    printf("    filevault info <input.vault>\n");
    printf("    filevault legacy-xor-encrypt <input>\n");
    printf("    filevault legacy-xor-decrypt <input.enc> [--key-file <path>]\n");
    printf("    filevault help\n");
    printf("    filevault version\n");
    printf("\n");
    printf("  Commands:\n");
    printf("    encrypt              Encrypt a file using AES-256-GCM\n");
    printf("    decrypt              Decrypt a .vault file\n");
    printf("    info                 Show encrypted file metadata\n");
    printf("    legacy-xor-encrypt   Encrypt using legacy XOR (insecure)\n");
    printf("    legacy-xor-decrypt   Decrypt legacy .enc files\n");
    printf("    help                 Show this help message\n");
    printf("    version              Show version information\n");
    printf("\n");
    printf("  Options:\n");
    printf("    -o <path>            Specify output file path\n");
    printf("    --force              Overwrite existing output file\n");
    printf("    --key-file <path>    Specify key file for legacy decrypt\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    filevault encrypt document.pdf\n");
    printf("    filevault decrypt document.pdf.vault\n");
    printf("    filevault encrypt photo.jpg -o photo.secure --force\n");
    printf("    filevault info secret.vault\n");
    printf("    filevault legacy-xor-decrypt old_file.enc\n");
    printf("\n");
    printf("  Security:\n");
    printf("    - Uses AES-256-GCM authenticated encryption\n");
    printf("    - Key derived via PBKDF2-HMAC-SHA256 (%d iterations)\n",
           FV_PBKDF2_ITERS);
    printf("    - Random salt and nonce per encryption\n");
    printf("    - Wrong password produces no output\n");
    printf("    - Tampered files are rejected\n");
    printf("\n");
}

void cmd_version(void) {
    printf("%s v%s\n", FV_APP_NAME, FV_VERSION_STRING);
    printf("Encryption: AES-256-GCM\n");
    printf("KDF: PBKDF2-HMAC-SHA256\n");
    printf("%s\n", FV_COPYRIGHT);
}
