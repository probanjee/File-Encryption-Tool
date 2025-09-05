#include <stdio.h>
#include <string.h>

// Decrypts file using XOR with same key used in encryption
void decryptFile(const char *filename, const char *key) {
    FILE *file = fopen(filename, "rb");  // Open encrypted file
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char outputFilename[100];
    snprintf(outputFilename, sizeof(outputFilename), "%s.dec", filename); // Decrypted file name

    FILE *outputFile = fopen(outputFilename, "wb");  // Create decrypted file
    if (outputFile == NULL) {
        perror("Error creating output file");
        fclose(file);
        return;
    }

    int keyLength = strlen(key);
    if (keyLength == 0) {
        printf("Decryption key cannot be empty!\n");
        fclose(file);
        fclose(outputFile);
        return;
    }

    int ch, i = 0;
    while ((ch = fgetc(file)) != EOF) {
        fputc(ch ^ key[i % keyLength], outputFile);  // XOR decryption logic (same as encryption)
        i++;
    }

    fclose(file);
    fclose(outputFile);

    printf("File decrypted successfully! Output file: %s\n", outputFilename);
}
