#include <stdio.h>
#include <string.h>
#include "decryption.h"

void decryptFile(const char *filename) {
    // Read key from key file
    char keyFilename[100];
    snprintf(keyFilename, sizeof(keyFilename), "%s.key", filename);
    FILE *keyFile = fopen(keyFilename, "r");
    if (!keyFile) {
        printf("Key file not found! Cannot decrypt %s\n", filename);
        return;
    }
    fscanf(keyFile, "%s", globalKey);
    fclose(keyFile);

    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char outputFilename[100];
    snprintf(outputFilename, sizeof(outputFilename), "%s.dec", filename);
    FILE *outputFile = fopen(outputFilename, "wb");
    if (!outputFile) {
        perror("Error creating output file");
        fclose(file);
        return;
    }

    int keyLength = strlen(globalKey);
    int ch, i = 0;
    while ((ch = fgetc(file)) != EOF) {
        fputc(ch ^ globalKey[i % keyLength], outputFile);
        i++;
    }

    fclose(file);
    fclose(outputFile);

    printf("File decrypted successfully! Output: %s\n", outputFilename);
}
