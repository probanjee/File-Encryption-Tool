/*
 * FileVault v2.0 — CLI Entry Point
 * main.c — Command-line argument parser and router
 *
 * Usage:
 *   filevault encrypt <input> [-o <output>] [--force]
 *   filevault decrypt <input.vault> [-o <output>] [--force]
 *   filevault info <input.vault>
 *   filevault legacy-xor-encrypt <input>
 *   filevault legacy-xor-decrypt <input.enc> [--key-file <path>]
 *   filevault help
 *   filevault version
 */

#include "cli_commands.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#endif

/* Enable ANSI escape codes on Windows 10+ */
static void enable_ansi_colors(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= 0x0004; /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
            SetConsoleMode(hOut, mode);
        }
    }
    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hErr != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hErr, &mode)) {
            mode |= 0x0004;
            SetConsoleMode(hErr, mode);
        }
    }
#endif
}

static int streq(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

int main(int argc, char *argv[]) {
    enable_ansi_colors();

    if (argc < 2) {
        cmd_help();
        return 1;
    }

    const char *command = argv[1];

    /* help / version — no additional args needed */
    if (streq(command, "help") || streq(command, "--help") || streq(command, "-h")) {
        cmd_help();
        return 0;
    }

    if (streq(command, "version") || streq(command, "--version") || streq(command, "-v")) {
        cmd_version();
        return 0;
    }

    /* encrypt <input> [-o <output>] [--force] */
    if (streq(command, "encrypt")) {
        if (argc < 3) {
            fprintf(stderr, "\n  [ERROR] Missing input file.\n");
            fprintf(stderr, "  Usage: filevault encrypt <input> [-o <output>] [--force]\n\n");
            return 1;
        }

        const char *input_path  = argv[2];
        const char *output_path = NULL;
        int force = 0;

        for (int i = 3; i < argc; i++) {
            if (streq(argv[i], "-o") && i + 1 < argc) {
                output_path = argv[++i];
            } else if (streq(argv[i], "--force")) {
                force = 1;
            } else {
                fprintf(stderr, "\n  [ERROR] Unknown option: %s\n\n", argv[i]);
                return 1;
            }
        }

        return cmd_encrypt(input_path, output_path, force);
    }

    /* decrypt <input.vault> [-o <output>] [--force] */
    if (streq(command, "decrypt")) {
        if (argc < 3) {
            fprintf(stderr, "\n  [ERROR] Missing input file.\n");
            fprintf(stderr, "  Usage: filevault decrypt <input.vault> [-o <output>] [--force]\n\n");
            return 1;
        }

        const char *input_path  = argv[2];
        const char *output_path = NULL;
        int force = 0;

        for (int i = 3; i < argc; i++) {
            if (streq(argv[i], "-o") && i + 1 < argc) {
                output_path = argv[++i];
            } else if (streq(argv[i], "--force")) {
                force = 1;
            } else {
                fprintf(stderr, "\n  [ERROR] Unknown option: %s\n\n", argv[i]);
                return 1;
            }
        }

        return cmd_decrypt(input_path, output_path, force);
    }

    /* info <input.vault> */
    if (streq(command, "info")) {
        if (argc < 3) {
            fprintf(stderr, "\n  [ERROR] Missing input file.\n");
            fprintf(stderr, "  Usage: filevault info <input.vault>\n\n");
            return 1;
        }
        return cmd_info(argv[2]);
    }

    /* legacy-xor-encrypt <input> */
    if (streq(command, "legacy-xor-encrypt")) {
        if (argc < 3) {
            fprintf(stderr, "\n  [ERROR] Missing input file.\n");
            fprintf(stderr, "  Usage: filevault legacy-xor-encrypt <input>\n\n");
            return 1;
        }
        return cmd_legacy_xor_encrypt(argv[2]);
    }

    /* legacy-xor-decrypt <input.enc> [--key-file <path>] */
    if (streq(command, "legacy-xor-decrypt")) {
        if (argc < 3) {
            fprintf(stderr, "\n  [ERROR] Missing input file.\n");
            fprintf(stderr, "  Usage: filevault legacy-xor-decrypt <input.enc> [--key-file <path>]\n\n");
            return 1;
        }

        const char *input_path    = argv[2];
        const char *key_file_path = NULL;

        for (int i = 3; i < argc; i++) {
            if (streq(argv[i], "--key-file") && i + 1 < argc) {
                key_file_path = argv[++i];
            } else {
                fprintf(stderr, "\n  [ERROR] Unknown option: %s\n\n", argv[i]);
                return 1;
            }
        }

        return cmd_legacy_xor_decrypt(input_path, key_file_path);
    }

    /* Unknown command */
    fprintf(stderr, "\n  [ERROR] Unknown command: %s\n", command);
    fprintf(stderr, "  Run 'filevault help' for usage information.\n\n");
    return 1;
}
