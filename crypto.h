#ifndef ENCRYPTION_H   // Prevent multiple inclusions of this header file
#define ENCRYPTION_H

// Global key variable (defined in set-key.c, accessible across modules)
extern char globalKey[200];

// Function Declarations (only declarations, implementations are in respective .c files)
void setKey();                        // Set a secret key for encryption/decryption
void encryptFile(const char *filename);  // Encrypt a file using the global key
void decryptFile(const char *filename);  // Decrypt a file using the global key

#endif // End of header guard
