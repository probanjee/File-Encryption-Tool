/*
 * FileVault v2.0 — test_wrong_password.c
 * Verify: wrong password → FV_ERR_AUTH_FAILED, no output file
 */

#include "test_framework.h"
#include "filevault/crypto_engine.h"
#include "filevault/file_utils.h"
#include <stdio.h>
#include <string.h>

static const char *TEST_INPUT    = "test_wrong_pw_input.txt";
static const char *TEST_ENC      = "test_wrong_pw_input.txt.vault";
static const char *TEST_DEC      = "test_wrong_pw_input.txt.dec";
static const char *CORRECT_PASS  = "CorrectPassword!";
static const char *WRONG_PASS    = "WrongPassword!";

static void create_test_file(const char *path, const char *content) {
    FILE *f = fopen(path, "wb");
    if (f) {
        fwrite(content, 1, strlen(content), f);
        fclose(f);
    }
}

static void test_wrong_password_fails(void) {
    create_test_file(TEST_INPUT, "Secret information that must remain protected.");

    /* Encrypt with correct password */
    FvEncryptRequest enc_req = {
        .input_path = TEST_INPUT, .output_path = TEST_ENC,
        .password = CORRECT_PASS, .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    FvError err = fv_encrypt_file(&enc_req, &result);
    TEST_ASSERT_EQ(err, FV_OK, "Encryption should succeed");

    /* Decrypt with WRONG password */
    FvDecryptRequest dec_req = {
        .input_path = TEST_ENC, .output_path = TEST_DEC,
        .password = WRONG_PASS, .force_overwrite = 1,
        .verify_only = 0, .progress_fn = NULL, .progress_data = NULL
    };
    err = fv_decrypt_file(&dec_req, &result);
    TEST_ASSERT_EQ(err, FV_ERR_AUTH_FAILED,
                   "Decryption with wrong password must fail with AUTH_FAILED");

    /* Verify no output file was created */
    TEST_ASSERT(!fv_file_exists(TEST_DEC),
                "No decrypted output file should exist after wrong password");

    remove(TEST_INPUT);
    remove(TEST_ENC);
    remove(TEST_DEC); /* Just in case */
}

static void test_similar_password_fails(void) {
    create_test_file(TEST_INPUT, "More secret data.");

    FvEncryptRequest enc_req = {
        .input_path = TEST_INPUT, .output_path = TEST_ENC,
        .password = "MyPassword123", .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    fv_encrypt_file(&enc_req, &result);

    /* Try with a very similar but different password */
    FvDecryptRequest dec_req = {
        .input_path = TEST_ENC, .output_path = TEST_DEC,
        .password = "MyPassword124", .force_overwrite = 1,
        .verify_only = 0, .progress_fn = NULL, .progress_data = NULL
    };
    FvError err = fv_decrypt_file(&dec_req, &result);
    TEST_ASSERT_EQ(err, FV_ERR_AUTH_FAILED,
                   "Even a similar password must fail");

    TEST_ASSERT(!fv_file_exists(TEST_DEC),
                "No output with similar wrong password");

    remove(TEST_INPUT);
    remove(TEST_ENC);
    remove(TEST_DEC);
}

int main(void) {
    printf("\n  === Wrong Password Tests ===\n\n");

    TEST_RUN(test_wrong_password_fails);
    TEST_RUN(test_similar_password_fails);

    TEST_SUMMARY();
    TEST_EXIT();
}
