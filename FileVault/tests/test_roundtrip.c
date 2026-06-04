/*
 * FileVault v2.0 — test_roundtrip.c
 * Verify: encrypt → decrypt → identical bytes
 */

#include "test_framework.h"
#include "filevault/crypto_engine.h"
#include "filevault/file_utils.h"
#include "filevault/secure_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TEST_INPUT   = "test_roundtrip_input.txt";
static const char *TEST_ENC     = "test_roundtrip_input.txt.vault";
static const char *TEST_DEC     = "test_roundtrip_input.txt.dec";
static const char *TEST_PASSWORD = "TestPassword123!";

static const char *SAMPLE_DATA =
    "The lion (Panthera leo) is a large cat of the genus Panthera, "
    "native to Sub-Saharan Africa and India. It has a muscular, "
    "broad-chested body; a short, rounded head; round ears; and a "
    "dark, hairy tuft at the tip of its tail.";

static void create_test_file(const char *path, const char *content) {
    FILE *f = fopen(path, "wb");
    if (f) {
        fwrite(content, 1, strlen(content), f);
        fclose(f);
    }
}

static int files_identical(const char *path_a, const char *path_b) {
    FILE *fa = fopen(path_a, "rb");
    FILE *fb = fopen(path_b, "rb");
    if (!fa || !fb) {
        if (fa) fclose(fa);
        if (fb) fclose(fb);
        return 0;
    }

    int identical = 1;
    int ca, cb;
    while (1) {
        ca = fgetc(fa);
        cb = fgetc(fb);
        if (ca != cb) { identical = 0; break; }
        if (ca == EOF) break;
    }

    fclose(fa);
    fclose(fb);
    return identical;
}

static void test_roundtrip_text(void) {
    /* Create test file */
    create_test_file(TEST_INPUT, SAMPLE_DATA);

    /* Encrypt */
    FvEncryptRequest enc_req = {
        .input_path      = TEST_INPUT,
        .output_path     = TEST_ENC,
        .password        = TEST_PASSWORD,
        .force_overwrite = 1,
        .progress_fn     = NULL,
        .progress_data   = NULL
    };
    FvResult enc_result;
    FvError err = fv_encrypt_file(&enc_req, &enc_result);
    TEST_ASSERT_EQ(err, FV_OK, "Encryption should succeed");

    /* Decrypt */
    FvDecryptRequest dec_req = {
        .input_path      = TEST_ENC,
        .output_path     = TEST_DEC,
        .password        = TEST_PASSWORD,
        .force_overwrite = 1,
        .verify_only     = 0,
        .progress_fn     = NULL,
        .progress_data   = NULL
    };
    FvResult dec_result;
    err = fv_decrypt_file(&dec_req, &dec_result);
    TEST_ASSERT_EQ(err, FV_OK, "Decryption should succeed");

    /* Compare */
    TEST_ASSERT(files_identical(TEST_INPUT, TEST_DEC),
                "Decrypted file should be byte-identical to original");

    /* Cleanup */
    remove(TEST_INPUT);
    remove(TEST_ENC);
    remove(TEST_DEC);
}

static void test_roundtrip_binary(void) {
    /* Create binary test file with all byte values */
    unsigned char binary_data[256];
    for (int i = 0; i < 256; i++) binary_data[i] = (unsigned char)i;

    FILE *f = fopen(TEST_INPUT, "wb");
    TEST_ASSERT(f != NULL, "Should create binary test file");
    if (f) {
        fwrite(binary_data, 1, 256, f);
        fclose(f);
    }

    FvEncryptRequest enc_req = {
        .input_path = TEST_INPUT, .output_path = TEST_ENC,
        .password = TEST_PASSWORD, .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    FvError err = fv_encrypt_file(&enc_req, &result);
    TEST_ASSERT_EQ(err, FV_OK, "Binary encryption should succeed");

    FvDecryptRequest dec_req = {
        .input_path = TEST_ENC, .output_path = TEST_DEC,
        .password = TEST_PASSWORD, .force_overwrite = 1,
        .verify_only = 0, .progress_fn = NULL, .progress_data = NULL
    };
    err = fv_decrypt_file(&dec_req, &result);
    TEST_ASSERT_EQ(err, FV_OK, "Binary decryption should succeed");

    TEST_ASSERT(files_identical(TEST_INPUT, TEST_DEC),
                "Binary roundtrip should produce identical output");

    remove(TEST_INPUT);
    remove(TEST_ENC);
    remove(TEST_DEC);
}

static void test_different_ciphertext(void) {
    /* Same file encrypted twice should produce different ciphertext
       (due to random salt + nonce) */
    create_test_file(TEST_INPUT, SAMPLE_DATA);

    FvEncryptRequest req = {
        .input_path = TEST_INPUT, .output_path = "enc_a.vault",
        .password = TEST_PASSWORD, .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    fv_encrypt_file(&req, &result);

    req.output_path = "enc_b.vault";
    fv_encrypt_file(&req, &result);

    TEST_ASSERT(!files_identical("enc_a.vault", "enc_b.vault"),
                "Two encryptions of the same file must produce different output");

    remove(TEST_INPUT);
    remove("enc_a.vault");
    remove("enc_b.vault");
}

int main(void) {
    printf("\n  === Roundtrip Tests ===\n\n");

    TEST_RUN(test_roundtrip_text);
    TEST_RUN(test_roundtrip_binary);
    TEST_RUN(test_different_ciphertext);

    TEST_SUMMARY();
    TEST_EXIT();
}
