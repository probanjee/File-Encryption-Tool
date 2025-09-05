#include <stdio.h>
#include <stdlib.h>
#include "encryption.h"
#include "decryption.h"
#include "set-key.h"

char globalKey[200]; // Global key variable

int main() {
    int choice;
    char filename[100];

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
                printf("Enter filename to encrypt: ");
                scanf("%s", filename);
                encryptFile(filename);
                break;

            case 2:
                printf("Enter filename to decrypt: ");
                scanf("%s", filename);
                decryptFile(filename); // Automatically reads key from .key
                break;

            case 3:
                setKey();  // Optional manual key
                break;

            case 4:
                exit(0);

            default:
                printf("Invalid choice! Try again.\n");
        }
    }

    return 0;
}
