/*
 * FileVault v2.0 — CLI Commands
 * cli_commands.h — Command handlers for encrypt, decrypt, info, legacy
 */

#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

/*
 * Each command handler returns 0 on success, non-zero on error.
 * Arguments are pre-parsed by main.c and passed as parameters.
 */

/* filevault encrypt <input> [-o <output>] [--force] */
int cmd_encrypt(const char *input_path, const char *output_path,
                int force_overwrite);

/* filevault decrypt <input.vault> [-o <output>] [--force] */
int cmd_decrypt(const char *input_path, const char *output_path,
                int force_overwrite);

/* filevault info <input.vault> */
int cmd_info(const char *input_path);

/* filevault legacy-xor-encrypt <input> */
int cmd_legacy_xor_encrypt(const char *input_path);

/* filevault legacy-xor-decrypt <input.enc> [--key-file <path>] */
int cmd_legacy_xor_decrypt(const char *input_path,
                           const char *key_file_path);

/* filevault help */
void cmd_help(void);

/* filevault version */
void cmd_version(void);

/* Print the FileVault banner */
void print_banner(void);

#endif /* CLI_COMMANDS_H */
