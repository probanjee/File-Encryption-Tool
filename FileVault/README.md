# FileVault v2.0 — Secure File Encryption Tool

A production-grade file encryption tool using **AES-256-GCM** authenticated encryption with **PBKDF2-HMAC-SHA256** key derivation. Built in C with a Qt 6 desktop GUI.

## Features

- **AES-256-GCM** authenticated encryption — industry standard
- **PBKDF2-HMAC-SHA256** key derivation (600,000 iterations)
- **Random salt and nonce** per encryption — same file encrypted twice produces different output
- **Tamper detection** — modified files are rejected
- **Streaming I/O** — supports files of any size without loading into RAM
- **Custom `.vault` container format** with versioned metadata
- **Legacy XOR mode** — backward compatible with v1.0 encrypted files
- **CLI + GUI** — command-line for automation, Qt desktop app for convenience
- **Cross-platform** — Windows, Linux, macOS

## Quick Start

### CLI

```bash
# Encrypt a file
filevault encrypt document.pdf

# Decrypt a file
filevault decrypt document.pdf.vault

# View encrypted file metadata
filevault info document.pdf.vault

# Decrypt legacy v1.0 XOR files
filevault legacy-xor-decrypt old_file.enc
```

### GUI

Launch `filevault-gui`, drag a file onto the window, enter a password, and click Encrypt or Decrypt.

## Security Model

- Encryption keys are **never stored** — derived from your password at runtime
- Passwords are **never saved** — wiped from memory immediately after use
- Wrong password produces **no output** — authentication tag verification prevents partial decryption
- Tampered files are **detected and rejected** — GCM authentication tag covers all ciphertext
- Each encryption uses a **unique random salt and nonce** — prevents ciphertext correlation

> **Warning:** If you forget your password, encrypted files **cannot be recovered**. There is no backdoor, no recovery key, and no password reset.

## Project Structure

```
FileVault/
├── core/           # C library — encryption engine
│   ├── include/    # Public headers
│   └── src/        # Implementation
├── cli/            # Command-line application
├── gui/            # Qt 6 desktop application
├── tests/          # Test suite
├── legacy/         # Original v1.0 code (reference)
├── docs/           # Additional documentation
└── examples/       # Sample test files
```

## Build Instructions

See [BUILD.md](BUILD.md) for complete build instructions for all platforms.

### Quick Build (Linux)

```bash
sudo apt install libssl-dev cmake build-essential
cmake -B build -DBUILD_TESTS=ON
cmake --build build
cd build && ctest --output-on-failure
```

## CLI Commands

| Command | Description |
|---|---|
| `filevault encrypt <file>` | Encrypt using AES-256-GCM |
| `filevault decrypt <file.vault>` | Decrypt a .vault file |
| `filevault info <file.vault>` | Show file metadata |
| `filevault legacy-xor-encrypt <file>` | Encrypt using legacy XOR (insecure) |
| `filevault legacy-xor-decrypt <file.enc>` | Decrypt legacy .enc files |
| `filevault help` | Show usage information |
| `filevault version` | Show version |

## Technical Details

| Parameter | Value |
|---|---|
| Encryption | AES-256-GCM (OpenSSL EVP) |
| Key Derivation | PBKDF2-HMAC-SHA256 |
| PBKDF2 Iterations | 600,000 |
| Salt Size | 16 bytes (128-bit) |
| Nonce Size | 12 bytes (96-bit) |
| Auth Tag Size | 16 bytes (128-bit) |
| Key Size | 32 bytes (256-bit) |
| Chunk Size | 1 MB (streaming) |

## License

MIT License — see [LICENSE](LICENSE) for details.

## Acknowledgments

- [OpenSSL](https://www.openssl.org/) for cryptographic primitives
- [Qt](https://www.qt.io/) for the GUI framework
- Original File Encryption Tool by the project author
