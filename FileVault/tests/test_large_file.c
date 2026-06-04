/*
 * FileVault v2.0 — test_large_file.c
 * Verify: Large file (10MB) streaming encryption without OOM
 */

#include "test_framework.h"
#include "filevault/crypto_engine.h"
#include "filevault/file_utils.h"
#include "filevault/password.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *LARGE_INPUT = "test_large_input.bin";
static const char *LARGE_ENC   = "test_large_input.bin.vault";
static const char *LARGE_DEC   = "test_large_input.bin.dec";
static const char *PASSWORD    = "LargeFileTestPassword!";

/* Generate a 10MB file with pseudo-random content */
static void create_large_file(const char *path, size_t size_mb) {
    FILE *f = fopen(path, "wb");
    if (!f) return;

    unsigned char buf[4096];
    size_t total = size_mb * 1024 * 1024;
    size_t written = 0;

    /* Simple PRNG for deterministic test data */
    unsigned int seed = 0xDEADBEEF;
    while (written < total) {
        size_t chunk = sizeof(buf);
        if (written + chunk > total) chunk = total - written;

        for (size_t i = 0; i < chunk; i++) {
            seed = seed * 1103515245 + 12345;
            buf[i] = (unsigned char)((seed >> 16) & 0xFF);
        }

        fwrite(buf, 1, chunk, f);
        written += chunk;
    }

    fclose(f);
}

static int files_identical(const char *a, const char *b) {
    FILE *fa = fopen(a, "rb");
    FILE *fb = fopen(b, "rb");
    if (!fa || !fb) { if (fa) fclose(fa); if (fb) fclose(fb); return 0; }

    unsigned char buf_a[8192], buf_b[8192];
    int identical = 1;

    while (1) {
        size_t ra = fread(buf_a, 1, sizeof(buf_a), fa);
        size_t rb = fread(buf_b, 1, sizeof(buf_b), fb);

        if (ra != rb) { identical = 0; break; }
        if (ra == 0) break;
        if (memcmp(buf_a, buf_b, ra) != 0) { identical = 0; break; }
    }

    fclose(fa);
    fclose(fb);
    return identical;
}

static void test_large_file_roundtrip(void) {
    printf("    Creating 10MB test file...\n");
    create_large_file(LARGE_INPUT, 10);

    int64_t input_size = fv_file_size(LARGE_INPUT);
    TEST_ASSERT(input_size > 10 * 1024 * 1024 - 100,
                "Large test file should be ~10MB");

    printf("    Encrypting 10MB file...\n");
    FvEncryptRequest enc_req = {
        .input_path = LARGE_INPUT, .output_path = LARGE_ENC,
        .password = PASSWORD, .force_overwrite = 1,
        .progress_fn = NULL, .progress_data = NULL
    };
    FvResult result;
    FvError err = fv_encrypt_file(&enc_req, &result);
    TEST_ASSERT_EQ(err, FV_OK, "Large file encryption should succeed");

    printf("    Decrypting 10MB file...\n");
    FvDecryptRequest dec_req = {
        .input_path = LARGE_ENC, .output_path = LARGE_DEC,
        .password = PASSWORD, .force_overwrite = 1,
        .verify_only = 0, .progress_fn = NULL, .progress_data = NULL
    };
    err = fv_decrypt_file(&dec_req, &result);
    TEST_ASSERT_EQ(err, FV_OK, "Large file decryption should succeed");

    printf("    Comparing 10MB files...\n");
    TEST_ASSERT(files_identical(LARGE_INPUT, LARGE_DEC),
                "Large file roundtrip must produce identical output");

    printf("    Encrypted in %.2fs, decrypted in total %.2fs\n",
           result.duration_seconds, result.duration_seconds);

    remove(LARGE_INPUT);
    remove(LARGE_ENC);
    remove(LARGE_DEC);
}

int main(void) {
    printf("\n  === Large File Tests ===\n\n");

    TEST_RUN(test_large_file_roundtrip);

    TEST_SUMMARY();
    TEST_EXIT();
}
