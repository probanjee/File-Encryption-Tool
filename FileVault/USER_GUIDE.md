# USER_GUIDE.md — FileVault v2.0

## Getting Started

FileVault encrypts files using AES-256-GCM, an industry-standard authenticated encryption algorithm. Your files are protected by a password you choose — no keys are stored anywhere.

## Command-Line Interface (CLI)

### Encrypt a File

```bash
filevault encrypt myfile.pdf
```

You will be prompted to enter and confirm a password. The encrypted file is saved as `myfile.pdf.vault`.

### Decrypt a File

```bash
filevault decrypt myfile.pdf.vault
```

Enter the same password used during encryption. The decrypted file is saved as `myfile.pdf.dec`.

### Custom Output Path

```bash
filevault encrypt report.docx -o secret_report.vault
filevault decrypt secret_report.vault -o report_restored.docx
```

### Force Overwrite

```bash
filevault encrypt myfile.pdf --force
```

### View File Info

```bash
filevault info myfile.pdf.vault
```

Shows encryption metadata without decrypting:

```
  Encrypted File Info
  ────────────────────────────────────────
  File          : myfile.pdf.vault
  Format        : FileVault Encrypted File
  Version       : 1
  Algorithm     : AES-256-GCM
  KDF           : PBKDF2-HMAC-SHA256
  Salt size     : 16 bytes
  Nonce size    : 12 bytes
  Tag size      : 16 bytes
  Original name : myfile.pdf
  Integrity     : Authenticated encryption (GCM)
  ────────────────────────────────────────
```

### Legacy Mode

If you have files encrypted with the original File Encryption Tool (v1.0 XOR):

```bash
filevault legacy-xor-decrypt King.txt.enc
```

This automatically finds the `.key` file and decrypts using legacy XOR. A warning is displayed because XOR encryption is not secure.

## GUI Application

### Main Window

1. **Select File**: Click "Browse..." or drag and drop a file onto the window
2. **Choose Mode**: Select "Encrypt" or "Decrypt" (auto-detected for `.vault` files)
3. **Enter Password**: Type your password. For encryption, confirm it twice
4. **Click Action**: Press "Encrypt File" or "Decrypt File"
5. **Watch Progress**: The progress bar shows speed and estimated time
6. **Check Results**: The activity log shows success or error details

### Themes

Go to **View → Toggle Theme** or **File → Settings** to switch between Dark and Light themes.

### Settings

- **Theme**: Dark or Light
- **Force Overwrite**: Automatically overwrite existing output files
- **Default Output Directory**: Where to save encrypted/decrypted files

## Supported File Types

FileVault can encrypt **any file type**:

| Category | Examples |
|---|---|
| Documents | .txt, .pdf, .docx, .xlsx, .pptx |
| Images | .jpg, .png, .gif, .bmp, .svg |
| Videos | .mp4, .mkv, .avi, .mov |
| Archives | .zip, .rar, .7z, .tar.gz |
| Code | .c, .py, .js, .html, .css |
| Binaries | .exe, .dll, .iso |
| Any other | All file types supported |

## Important Notes

### Password Safety

- **There is no password recovery**. If you forget your password, the file cannot be decrypted.
- Use a strong, memorable password.
- Minimum password length: 4 characters (longer is better).

### File Safety

- The original file is **not modified or deleted** during encryption.
- You can safely delete the original after verifying the encrypted file.
- Each encryption produces a **different output** (due to random salt/nonce), even with the same password.

### Error Messages

| Message | Meaning |
|---|---|
| "Authentication failed" | Wrong password or the file was tampered with |
| "File is not a valid FileVault encrypted file" | The file wasn't created by FileVault |
| "Output file already exists" | Use `--force` or choose a different output path |
| "Password cannot be empty" | You must enter a password |
| "Passwords do not match" | Confirm password didn't match during encryption |

## FAQ

**Q: Can I encrypt folders?**
A: Not directly. Create a zip/tar archive first, then encrypt the archive.

**Q: Is XOR encryption secure?**
A: No. Legacy XOR mode exists only to decrypt old files. Always use the default AES-256-GCM mode for new encryption.

**Q: Can I recover a file if I forget the password?**
A: No. This is by design — there is no backdoor.

**Q: Why does encrypting the same file twice produce different output?**
A: A unique random salt and nonce are generated for each encryption. This is a security feature that prevents attackers from detecting identical files.
