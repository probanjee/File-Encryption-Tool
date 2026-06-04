/*
 * FileVault v2.0 — test_legacy_xor.c
 * Verify: Legacy XOR mode backward compatibility
 */

#include "test_framework.h"
#include "filevault/legacy_xor.h"
#include "filevault/file_utils.h"
#include "filevault/secure_mem.h"
#include <stdio.h>
#include <string.h>

static const char *TEST_INPUT = "test_legacy_input.txt";
static const char *LEGACY_KEY = "Prosun";

static const char *SAMPLE_DATA =
    "The lion (Panthera leo) is a large cat of the genus Panthera.";

static void create_test_file(const char *path, const char *content) {
    FILE *f = fopen(path, "wb");
    if (f) { fwrite(content, 1, strlen(content), f); fclose(f); }
}

static int files_identical(const char *a, const char *b) {
    FILE *fa = fopen(a, "rb");
    FILE *fb = fopen(b, "rb");
    if (!fa || !fb) { if (fa) fclose(fa); if (fb) fclose(fb); return 0; }
    int identical = 1;
    int ca, cb;
    while (1) {
        ca = fgetc(fa); cb = fgetc(fb);
        if (ca != cb) { identical = 0; break; }
        if (ca == EOF) break;
    }
    fclose(fa); fclose(fb);
    return identical;
}

static void test_legacy_xor_roundtrip(void) {
    create_test_file(TEST_INPUT, SAMPLE_DATA);

    /* Encrypt using legacy XOR */
    FvError err = fv_legacy_xor_encrypt_auto(TEST_INPUT, LEGACY_KEY,
                                             strlen(LEGACY_KEY), NULL, NULL);
    TEST_ASSERT_EQ(err, FV_OK, "Legacy XOR encrypt should succeed");

    /* Verify .enc and .key files created */
    TEST_ASSERT(fv_file_exists("test_legacy_input.txt.enc"),
                "Legacy .enc file should exist");
    TEST_ASSERT(fv_file_exists("test_legacy_input.txt.key"),
                "Legacy .key file should exist");

    /* Decrypt using legacy XOR (auto key detection) */
    err = fv_legacy_xor_decrypt("test_legacy_input.txt.enc",
                                NULL, NULL, NULL, NULL);
    TEST_ASSERT_EQ(err, FV_OK, "Legacy XOR decrypt should succeed");

    /* Verify decrypted output matches original */
    TEST_ASSERT(fv_file_exists("test_legacy_input.txt.dec"),
                "Legacy .dec file should exist");
    TEST_ASSERT(files_identical(TEST_INPUT, "test_legacy_input.txt.dec"),
                "Legacy XOR roundtrip should produce identical output");

    remove(TEST_INPUT);
    remove("test_legacy_input.txt.enc");
    remove("test_legacy_input.txt.key");
    remove("test_legacy_input.txt.dec");
}

static void test_legacy_key_path_fix(void) {
    /* Verify the key path bug is fixed:
       "file.enc" should look for "file.key", NOT "file.enc.key" */
    char key_path[256];
    FvError err = fv_generate_legacy_key_path("King.txt.enc",
                                              key_path, sizeof(key_path));
    TEST_ASSERT_EQ(err, FV_OK, "Key path generation should succeed");
    TEST_ASSERT_STR_EQ(key_path, "King.txt.key",
                       "Key path must be King.txt.key, NOT King.txt.enc.key");
}

static void test_legacy_output_path_fix(void) {
    /* Verify output naming bug is fixed:
       "file.enc" should produce "file.dec", NOT "file.enc.dec" */
    char output_path[256];
    FvError err = fv_generate_legacy_decrypt_output("King.txt.enc",
                                                    output_path,
                                                    sizeof(output_path));
    TEST_ASSERT_EQ(err, FV_OK, "Output path generation should succeed");
    TEST_ASSERT_STR_EQ(output_path, "King.txt.dec",
                       "Output must be King.txt.dec, NOT King.txt.enc.dec");
}

static void test_legacy_empty_key_rejected(void) {
    create_test_file(TEST_INPUT, "Data");

    FvError err = fv_legacy_xor_encrypt_auto(TEST_INPUT, "", 0, NULL, NULL);
    TEST_ASSERT_EQ(err, FV_ERR_EMPTY_PASSWORD,
                   "Legacy XOR with empty key must fail");

    remove(TEST_INPUT);
}

int main(void) {
    printf("\n  === Legacy XOR Tests ===\n\n");

    TEST_RUN(test_legacy_xor_roundtrip);
    TEST_RUN(test_legacy_key_path_fix);
    TEST_RUN(test_legacy_output_path_fix);
    TEST_RUN(test_legacy_empty_key_rejected);

    TEST_SUMMARY();
    TEST_EXIT();
}
