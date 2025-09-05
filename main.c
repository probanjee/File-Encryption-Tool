#include <stdio.h>   // For standard input/output functions
#include <stdlib.h>  // For exit()
#include <string.h>  // For handling strings

// Custom headers
#include "encryption.h"
#include "decryption.h"
#include "set_key.h"

int main() {
    int choice;
    char filename[100], key[100];

    while (1) {
        printf("\n--- File Encryption Tool ---\n");
        printf("1. Encrypt File\n");
        printf("2. Decrypt File\n");
        printf("3. Set Key\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter the filename to encrypt: ");
                scanf("%s", filename);
                printf("Enter the encryption key: ");
                scanf("%s", key);
                encryptFile(filename, key);
                break;

            case 2:
                printf("Enter the filename to decrypt: ");
                scanf("%s", filename);
                printf("Enter the decryption key: ");
                scanf("%s", key);
                decryptFile(filename, key);
                break;

            case 3:
                setKey();  // Secure key setup
                break;

            case 4:
                exit(0);

            default:
                printf("Invalid choice! Please try again.\n");
        }
    }

    return 0;
}
