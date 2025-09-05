# File-Encryption-Tool
A lightweight File Encryption Tool in C that secures files using XOR-based symmetric encryption. Supports encryption, decryption, and custom key management. Cross-platform, fast, and modular design ensures data confidentiality while preventing unauthorized file access.
## 🚀 Features
- Encrypt any file with a user-defined key 🔑
- Decrypt files only with the same key 🔓
- Key can be a number, word, or combination of characters
- Lightweight XOR-based encryption (fast & simple)
- Modular structure for better readability (separate files for encryption, decryption, and key management)

---

## 📂 Project Structure
FileEncryptionTool/
│── main.c # Main menu-driven program
│── encryption.c # Encryption logic
│── decryption.c # Decryption logic
│── set-key.c # Key setting functionality
│── README.md # Project documentation

---

## 🧩 Algorithm & Logic
1. User provides a **filename** and a **key**.
2. Each byte of the file is **XORed** with the corresponding byte of the key.
3. Encrypted output is saved as `<filename>.enc`.
4. For decryption, the same XOR operation is applied with the **same key**.
5. Decrypted output is saved as `<filename>.dec`.
6. If the wrong key is used → decrypted file becomes unreadable (corrupted).

---

## ⚙️ How to Compile & Run
### On Linux / macOS:

gcc main.c encryption.c decryption.c set-key.c -o FileEncryptionTool
./FileEncryptionTool

### On Windows (MinGW / MSYS2 / WSL):

gcc main.c encryption.c decryption.c set-key.c -o FileEncryptionTool.exe
FileEncryptionTool.exe

📸 Example Usage:

--- File Encryption Tool ---
1. Encrypt File
2. Decrypt File
3. Set Key
4. Exit
Enter your choice: 1
Enter the filename to encrypt: secret.txt
Enter the encryption key: prosun
File encrypted successfully! Output file: secret.txt.enc

📌 Future Improvements

Save key securely in an external file or database

Add support for multiple encryption algorithms (AES, DES, etc.)

Cross-platform GUI version
