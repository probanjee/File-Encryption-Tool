# SECURITY.md — FileVault v2.0 Security Documentation

## Cryptographic Design

### Encryption Algorithm
**AES-256-GCM** (Galois/Counter Mode) via OpenSSL EVP API.

- 256-bit key provides 128-bit security level
- GCM mode provides both confidentiality and authenticity in a single pass
- 12-byte (96-bit) nonce — the recommended size for GCM
- 16-byte (128-bit) authentication tag appended to ciphertext

### Key Derivation
**PBKDF2-HMAC-SHA256** with 600,000 iterations (per OWASP 2024 recommendation).

- 16-byte (128-bit) random salt per file
- Salt stored in the `.vault` file header (not secret)
- Output: 32-byte (256-bit) derived key
- Salt ensures identical passwords produce different keys for different files

### Random Number Generation
**OpenSSL `RAND_bytes()`** for all random values:
- Salt generation
- Nonce/IV generation

This uses the operating system's CSPRNG (CryptGenRandom on Windows, /dev/urandom on Linux, SecRandomCopyBytes on macOS).

## Threat Model

### What FileVault Protects Against

| Threat | Protection |
|---|---|
| Unauthorized file access | AES-256-GCM encryption |
| Password guessing (brute force) | PBKDF2 with 600K iterations |
| File tampering/modification | GCM authentication tag |
| Password/key theft from disk | No passwords or keys are ever stored |
| Memory snooping | Secure memory wipe after use |
| Ciphertext correlation | Unique random salt + nonce per encryption |

### What FileVault Does NOT Protect Against

| Threat | Reason |
|---|---|
| Keyloggers capturing password | Application-level protection — use OS security |
| Physical access to running system | Memory forensics during encryption/decryption |
| Lost/forgotten passwords | No recovery mechanism exists by design |
| Quantum computing (future) | AES-256 offers 128-bit post-quantum security for symmetric encryption, but KDF may need future upgrades |
| Side-channel attacks | Not hardened against timing attacks on the CPU level |
| Rubber-hose cryptanalysis | No technical solution exists |

## Security Properties

### Authenticated Encryption
- Decryption verifies the authentication tag **before** producing any output
- If verification fails (wrong password or tampered file), **no output file is created**
- Partial decryption is not possible — the entire ciphertext is authenticated

### No Key Storage
- Encryption keys exist only in memory during operation
- Keys are derived from the user's password at runtime
- After operation completes, all key material is securely zeroed using `fv_secure_zero()`

### Secure Memory Handling
- `fv_secure_zero()` uses platform-specific APIs guaranteed not to be optimized away:
  - Windows: `SecureZeroMemory()`
  - Linux: `explicit_bzero()`
  - macOS: `memset_s()` or compiler barrier
  - Fallback: volatile function pointer technique
- Password buffers are wiped immediately after key derivation
- Derived keys are wiped immediately after encryption/decryption

### Nonce Safety
- 12-byte random nonce per encryption via `RAND_bytes()`
- Combined with unique salt, the probability of nonce reuse is negligible
- AES-256-GCM nonce collision probability: 2^(-48) per salt (birthday bound at ~2^48 encryptions)

## File Format Security

The `.vault` file format includes:
- **Magic bytes** to prevent accidental processing of non-vault files
- **Version field** for future-proof format upgrades
- **Algorithm and KDF identifiers** for forward compatibility
- **Authentication tag at end of file** — covers all ciphertext

See `docs/VAULT_FORMAT.md` for the complete binary specification.

## Known Limitations

1. **PBKDF2 vs. Argon2id**: PBKDF2 is used instead of Argon2id because OpenSSL doesn't natively support Argon2. Argon2id would provide better resistance against GPU/ASIC attacks. Future versions may add libsodium support for Argon2id.

2. **Single-file encryption**: FileVault encrypts individual files. Directory encryption requires encrypting each file separately or creating an archive first.

3. **No key file mode**: Authentication is password-only. Hardware security key or key file support is not implemented.

4. **Legacy XOR mode**: The legacy mode is intentionally insecure and exists only for backward compatibility with v1.0 files. It should never be used for new encryption.

## Responsible Disclosure

If you discover a security vulnerability in FileVault, please report it responsibly. Do not open a public issue. Contact the maintainer directly.

## Security Checklist for Contributors

- [ ] Never log or print passwords or derived keys
- [ ] Always call `fv_secure_zero()` on sensitive buffers before freeing
- [ ] Always use `RAND_bytes()` for random values — never `rand()` or `random()`
- [ ] Never use ECB mode or unauthenticated encryption
- [ ] Always validate the authentication tag before writing decrypted output
- [ ] Always write decrypted data to a temp file first, rename only after auth succeeds
- [ ] Never store passwords or raw keys in any file
