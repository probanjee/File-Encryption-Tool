#include <stdio.h>
#include <string.h>

// Function for setting a secure key (future upgrade can save to secure storage)
void setKey() {
    char key[100];
    printf("Enter a new key: ");
    scanf("%s", key);

    // Placeholder for key storage logic
    printf("Key set successfully! (Future: Store securely)\n");
}
