/*
 * FileVault v2.0 — test_tampered.c
 * Verify: tampered ciphertext → FV_ERR_AUTH_FAILED
 */

#include "test_framework.h"
#include "filevault/crypto_engine.h"
#include "filevault/file_utils.h"
#include <stdio.h>
#include <string.h>

static const char *TEST_INPUT = "test_tamper_input.txt";
static const char *TEST_ENC   = "test_tamper_input.txt.vault";
static const char *TEST_DEC   = "test_tamper_input.txt.dec";
static const char *PASSWORD   = "TamperTestPass!";

static void create_test_file(const char *path, const char *content) {
    FILE *f = fopen(path, "wb");
    if (f) { fwrite(content, 1, strlen(content), f); fclose(f); }
}

static void flip_byte_in_file(const char *path, long offset) {
    FILE *f = fopen(path, "r+b");
    if (!f) return;
    fseek(f, offset, SEEK_SET);
    unsigned char byte;
    if (fread(&byte, 1, 1, f) == 1) {
        byte ^= 0xFF;
        fseek(f, offset, SEEK_SET);
        fwrite(&byte, 1, 1, f);
    }
    fclose(f);
}

static void test_tampered_ciphertext(void) {
    create_test_file(TEST_INPUT, "Data that will be tampered with.");

    FvEncryptRequest enc_req = {
        .input_path = TEST_INPUT, .output_path = TEST_ENC,
        .password = PASSWORD, .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    fv_encrypt_file(&enc_req, &result);

    /* Flip a byte in the ciphertext area (past the header) */
    /* Header size is roughly 69 bytes for this filename. We tamper at offset 75. */
    long tamper_offset = 75;
    flip_byte_in_file(TEST_ENC, tamper_offset);

    /* Attempt decryption */
    FvDecryptRequest dec_req = {
        .input_path = TEST_ENC, .output_path = TEST_DEC,
        .password = PASSWORD, .force_overwrite = 1,
        .verify_only = 0, .progress_fn = NULL, .progress_data = NULL
    };
    FvError err = fv_decrypt_file(&dec_req, &result);
    TEST_ASSERT_EQ(err, FV_ERR_AUTH_FAILED,
                   "Tampered ciphertext must fail authentication");

    TEST_ASSERT(!fv_file_exists(TEST_DEC),
                "No output after tampered ciphertext");

    remove(TEST_INPUT);
    remove(TEST_ENC);
    remove(TEST_DEC);
}

static void test_tampered_tag(void) {
    create_test_file(TEST_INPUT, "Data with tag tampering.");

    FvEncryptRequest enc_req = {
        .input_path = TEST_INPUT, .output_path = TEST_ENC,
        .password = PASSWORD, .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    fv_encrypt_file(&enc_req, &result);

    /* Flip last byte (part of auth tag) */
    int64_t file_size = fv_file_size(TEST_ENC);
    flip_byte_in_file(TEST_ENC, (long)(file_size - 1));

    FvDecryptRequest dec_req = {
        .input_path = TEST_ENC, .output_path = TEST_DEC,
        .password = PASSWORD, .force_overwrite = 1,
        .verify_only = 0, .progress_fn = NULL, .progress_data = NULL
    };
    FvError err = fv_decrypt_file(&dec_req, &result);
    TEST_ASSERT_EQ(err, FV_ERR_AUTH_FAILED,
                   "Tampered auth tag must fail");

    TEST_ASSERT(!fv_file_exists(TEST_DEC),
                "No output after tag tampering");

    remove(TEST_INPUT);
    remove(TEST_ENC);
    remove(TEST_DEC);
}

static void test_tampered_header(void) {
    create_test_file(TEST_INPUT, "Data with header tampering.");

    FvEncryptRequest enc_req = {
        .input_path = TEST_INPUT, .output_path = TEST_ENC,
        .password = PASSWORD, .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    fv_encrypt_file(&enc_req, &result);

    /* Flip a byte in the header (salt area at offset ~22) */
    flip_byte_in_file(TEST_ENC, 22);

    FvDecryptRequest dec_req = {
        .input_path = TEST_ENC, .output_path = TEST_DEC,
        .password = PASSWORD, .force_overwrite = 1,
        .verify_only = 0, .progress_fn = NULL, .progress_data = NULL
    };
    FvError err = fv_decrypt_file(&dec_req, &result);

    /* Tampered salt → derived key is different → auth fails */
    TEST_ASSERT_EQ(err, FV_ERR_AUTH_FAILED,
                   "Tampered header/salt must fail authentication");

    remove(TEST_INPUT);
    remove(TEST_ENC);
    remove(TEST_DEC);
}

int main(void) {
    printf("\n  === Tampered File Tests ===\n\n");

    TEST_RUN(test_tampered_ciphertext);
    TEST_RUN(test_tampered_tag);
    TEST_RUN(test_tampered_header);

    TEST_SUMMARY();
    TEST_EXIT();
}
