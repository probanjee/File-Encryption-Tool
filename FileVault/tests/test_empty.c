/*
 * FileVault v2.0 — test_empty.c
 * Verify: empty password rejected, empty file rejected
 */

#include "test_framework.h"
#include "filevault/crypto_engine.h"
#include "filevault/file_utils.h"
#include "filevault/password.h"
#include <stdio.h>
#include <string.h>

static const char *TEST_INPUT = "test_empty_input.txt";
static const char *TEST_ENC   = "test_empty_input.txt.vault";
static const char *EMPTY_FILE = "test_empty_file.txt";

static void create_test_file(const char *path, const char *content) {
    FILE *f = fopen(path, "wb");
    if (f) { fwrite(content, 1, strlen(content), f); fclose(f); }
}

static void create_empty_file(const char *path) {
    FILE *f = fopen(path, "wb");
    if (f) fclose(f);
}

static void test_empty_password_rejected(void) {
    create_test_file(TEST_INPUT, "Some data to encrypt.");

    FvEncryptRequest req = {
        .input_path = TEST_INPUT, .output_path = TEST_ENC,
        .password = "", .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    FvError err = fv_encrypt_file(&req, &result);
    TEST_ASSERT_EQ(err, FV_ERR_EMPTY_PASSWORD,
                   "Empty password must be rejected for encryption");

    remove(TEST_INPUT);
    remove(TEST_ENC);
}

static void test_short_password_rejected(void) {
    create_test_file(TEST_INPUT, "Data.");

    FvEncryptRequest req = {
        .input_path = TEST_INPUT, .output_path = TEST_ENC,
        .password = "ab", .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    FvError err = fv_encrypt_file(&req, &result);
    TEST_ASSERT_EQ(err, FV_ERR_PASSWORD_TOO_SHORT,
                   "Short password must be rejected");

    remove(TEST_INPUT);
    remove(TEST_ENC);
}

static void test_empty_file_rejected(void) {
    create_empty_file(EMPTY_FILE);

    FvEncryptRequest req = {
        .input_path = EMPTY_FILE, .output_path = "empty.vault",
        .password = "ValidPassword123", .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    FvError err = fv_encrypt_file(&req, &result);
    TEST_ASSERT_EQ(err, FV_ERR_FILE_EMPTY,
                   "Empty file must be rejected");

    remove(EMPTY_FILE);
    remove("empty.vault");
}

static void test_missing_file_rejected(void) {
    FvEncryptRequest req = {
        .input_path = "nonexistent_file_xyz.txt", .output_path = "out.vault",
        .password = "ValidPassword123", .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    FvError err = fv_encrypt_file(&req, &result);
    TEST_ASSERT_EQ(err, FV_ERR_FILE_NOT_FOUND,
                   "Missing file must return FILE_NOT_FOUND");
}

static void test_password_validation(void) {
    TEST_ASSERT_EQ(fv_validate_password(NULL, 0), FV_ERR_EMPTY_PASSWORD,
                   "NULL password rejected");
    TEST_ASSERT_EQ(fv_validate_password("", 0), FV_ERR_EMPTY_PASSWORD,
                   "Empty string password rejected");
    TEST_ASSERT_EQ(fv_validate_password("ab", 2), FV_ERR_PASSWORD_TOO_SHORT,
                   "2-char password rejected");
    TEST_ASSERT_EQ(fv_validate_password("abcd", 4), FV_OK,
                   "4-char password accepted");
    TEST_ASSERT_EQ(fv_validate_password("StrongPass!123", 14), FV_OK,
                   "Strong password accepted");
}

int main(void) {
    printf("\n  === Empty/Validation Tests ===\n\n");

    TEST_RUN(test_empty_password_rejected);
    TEST_RUN(test_short_password_rejected);
    TEST_RUN(test_empty_file_rejected);
    TEST_RUN(test_missing_file_rejected);
    TEST_RUN(test_password_validation);

    TEST_SUMMARY();
    TEST_EXIT();
}
