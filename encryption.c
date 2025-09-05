#include <stdio.h>
#include <string.h>
#include "encryption.h"

void encryptFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char outputFilename[100];
    snprintf(outputFilename, sizeof(outputFilename), "%s.enc", filename);
    FILE *outputFile = fopen(outputFilename, "wb");
    if (!outputFile) {
        perror("Error creating output file");
        fclose(file);
        return;
    }

    int keyLength = strlen(globalKey);
    if (keyLength == 0) {
        printf("Encryption key not set! Please set key first.\n");
        fclose(file);
        fclose(outputFile);
        return;
    }

    int ch, i = 0;
    while ((ch = fgetc(file)) != EOF) {
        fputc(ch ^ globalKey[i % keyLength], outputFile);
        i++;
    }

    fclose(file);
    fclose(outputFile);

    // Save key automatically in a .key file
    char keyFilename[100];
    snprintf(keyFilename, sizeof(keyFilename), "%s.key", filename);
    FILE *keyFile = fopen(keyFilename, "w");
    if (keyFile) {
        fprintf(keyFile, "%s", globalKey);
        fclose(keyFile);
    }

    printf("File encrypted successfully! Output: %s\n", outputFilename);
    printf("Encryption key saved in: %s\n", keyFilename);
}
