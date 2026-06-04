# ARCHITECTURE.md — FileVault v2.0 System Architecture

## Module Dependency Diagram

```
┌─────────────────────────────────────────────────┐
│                  Applications                    │
│                                                  │
│   ┌──────────┐          ┌──────────────────┐    │
│   │  CLI     │          │  Qt GUI          │    │
│   │  main.c  │          │  MainWindow.cpp  │    │
│   │  cmds.c  │          │  Worker.cpp      │    │
│   │  prog.c  │          │  Settings.cpp    │    │
│   └────┬─────┘          └────────┬─────────┘    │
│        │                         │               │
└────────┼─────────────────────────┼───────────────┘
         │                         │
         └──────────┬──────────────┘
                    │
         ┌──────────▼──────────────────────────────┐
         │          Core Library (C)                │
         │          filevault_core.a                │
         │                                          │
         │  ┌──────────────┐  ┌─────────────────┐  │
         │  │ crypto_      │  │ file_format     │  │
         │  │ engine       │  │ .vault R/W      │  │
         │  │ AES-256-GCM  │  │ header parse    │  │
         │  └──────┬───────┘  └────────┬────────┘  │
         │         │                    │           │
         │  ┌──────▼───────┐  ┌────────▼────────┐  │
         │  │ password     │  │ file_utils      │  │
         │  │ PBKDF2-SHA256│  │ path helpers    │  │
         │  │ echo-off     │  │ validation      │  │
         │  └──────┬───────┘  └────────┬────────┘  │
         │         │                    │           │
         │  ┌──────▼───────┐  ┌────────▼────────┐  │
         │  │ secure_mem   │  │ errors          │  │
         │  │ zero wipe    │  │ codes + msgs    │  │
         │  └──────────────┘  └─────────────────┘  │
         │                                          │
         │  ┌──────────────┐                        │
         │  │ legacy_xor   │  (backward compat)     │
         │  │ XOR encrypt  │                        │
         │  └──────────────┘                        │
         │                                          │
         └──────────────────────────────────────────┘
                    │
         ┌──────────▼──────────────────────────────┐
         │          External Dependencies           │
         │                                          │
         │  ┌──────────────┐  ┌─────────────────┐  │
         │  │ OpenSSL 3.x  │  │ Qt 6 (optional) │  │
         │  │ libcrypto    │  │ Widgets          │  │
         │  └──────────────┘  └─────────────────┘  │
         └──────────────────────────────────────────┘
```

## Data Flow: Encryption

```
User Password ──► PBKDF2(password, salt) ──► 256-bit Key
                         │
Random Salt ─────────────┘
Random Nonce ──────────────────────────────────────┐
                                                   │
Input File ──► [Read 1MB chunk] ──► AES-256-GCM ──► Ciphertext ──► .vault file
                                   Encrypt          │
                                                   ▼
                                              Auth Tag ──► appended to .vault
```

## Data Flow: Decryption

```
.vault file ──► Read Header ──► Extract Salt, Nonce
                    │
User Password ──► PBKDF2(password, salt) ──► 256-bit Key
                                                │
Read Auth Tag (from EOF-16) ────────────────────┤
                                                │
Ciphertext ──► [Read 1MB chunk] ──► AES-256-GCM ──► Plaintext ──► .tmp file
                                    Decrypt
                                       │
                                  Verify Tag
                                       │
                              ┌────────▼────────┐
                              │ Tag Valid?       │
                              ├─────┬────────────┤
                              │ Yes │ No         │
                              │     │            │
                              ▼     ▼            │
                         Rename    Delete .tmp   │
                         .tmp →    Return ERROR  │
                         output                  │
                              └──────────────────┘
```

## Thread Model (GUI)

```
┌──────────────────────┐     signals      ┌────────────────────┐
│ Main Thread (GUI)    │ ◄──────────────── │ Worker Thread      │
│                      │                   │                    │
│ - MainWindow         │ progressChanged() │ - EncryptionWorker │
│ - User interaction   │ finished()        │ - fv_encrypt_file  │
│ - Progress updates   │ error()           │ - fv_decrypt_file  │
│ - Settings/About     │                   │                    │
└──────────────────────┘                   └────────────────────┘
```
