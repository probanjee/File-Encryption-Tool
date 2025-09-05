#include <stdio.h>
#include <string.h>
#include "set-key.h"

// Optional: user can manually set key
void setKey() {
    printf("Enter a new key: ");
    scanf("%s", globalKey);
    printf("Key set successfully!\n");
}
