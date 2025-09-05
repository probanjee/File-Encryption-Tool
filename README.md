# File Encryption Tool

A lightweight **File Encryption Tool in C** that secures files using **XOR-based symmetric encryption**.  
Supports encryption, decryption, and automatic key management.  
Cross-platform, fast, and modular design ensures data confidentiality while preventing unauthorized access.

---

## 🚀 Features

- Encrypt any file with a **user-defined key** 🔑  
- Decrypt files **only with the key used during encryption** 🔓  
- Key automatically saved in `<filename>.key` for future decryption  
- Lightweight XOR-based encryption (fast & simple)  
- Modular structure (separate files for encryption, decryption, key management)  

---

## 📂 Project Structure

FileEncryptionTool/
│── main.c
│── encryption.c
│── decryption.c
│── set-key.c
│── encryption.h
│── decryption.h
│── set-key.h
│── README.md



---

## 🧩 Algorithm & Logic

1. User chooses action: **set key, encrypt, decrypt**  
2. For encryption:
   - File contents are XORed with `globalKey`.  
   - Encrypted file is saved as `<filename>.enc`.  
   - Key is automatically saved in `<filename>.key`.  
3. For decryption:
   - Program reads key from `<filename>.key`.  
   - XOR operation restores original content in `<filename>.dec`.  
   - Wrong/missing key → decryption fails.  

---

## ⚙️ How to Compile & Run

**Linux / macOS:**

gcc main.c encryption.c decryption.c set-key.c -o FileEncryptionTool
./FileEncryptionTool

**Windows (MinGW / MSYS2 / WSL):**

gcc main.c encryption.c decryption.c set-key.c -o FileEncryptionTool.exe

then run this...

.\FileEncryptionTool.exe

📌 **Future Improvements**

-  Encrypt key file instead of plain text

-  Support multiple encryption algorithms (AES, DES, etc.)

-  Cross-platform GUI version
