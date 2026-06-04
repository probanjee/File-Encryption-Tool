# BUILD.md — FileVault v2.0 Build Instructions

## Prerequisites

- **CMake** ≥ 3.20
- **C17-capable compiler** (GCC ≥ 9, Clang ≥ 10, or MSVC 2019+)
- **OpenSSL** ≥ 3.0 (required for AES-256-GCM and PBKDF2)
- **Qt 6** (optional, only needed for GUI)

---

## Windows

### Option A: MSVC + vcpkg

```powershell
# Install OpenSSL via vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install openssl:x64-windows

# Build FileVault
cd path\to\FileVault
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release

# Run tests
cd build
ctest -C Release --output-on-failure
```

### Option B: MSYS2 + MinGW

```bash
# Install dependencies
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-openssl

# Build
cmake -B build -G "MinGW Makefiles"
cmake --build build

# Run tests
cd build && ctest --output-on-failure
```

### With Qt 6 GUI (Windows)

```powershell
# Install Qt 6 via Qt Online Installer or vcpkg
vcpkg install qt6:x64-windows

# Build with GUI
cmake -B build -DBUILD_GUI=ON -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

---

## Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake libssl-dev

# Build
cd FileVault
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run tests
cd build && ctest --output-on-failure

# Install (optional)
sudo cmake --install build
```

### With Qt 6 GUI (Linux)

```bash
sudo apt install qt6-base-dev

cmake -B build -DBUILD_GUI=ON -DBUILD_TESTS=ON
cmake --build build
```

---

## macOS

```bash
# Install dependencies
brew install openssl cmake

# Build (must point to Homebrew OpenSSL)
cmake -B build -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)
cmake --build build

# Run tests
cd build && ctest --output-on-failure
```

### With Qt 6 GUI (macOS)

```bash
brew install qt@6

cmake -B build -DBUILD_GUI=ON -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)
cmake --build build
```

---

## Build Options

| Option | Default | Description |
|---|---|---|
| `BUILD_GUI` | `OFF` | Build the Qt 6 GUI application |
| `BUILD_TESTS` | `ON` | Build the test suite |

---

## Output Binaries

After building, the following binaries are produced:

| Binary | Location | Description |
|---|---|---|
| `filevault` | `build/cli/filevault` | CLI application |
| `filevault-gui` | `build/gui/filevault-gui` | Qt GUI application (if `BUILD_GUI=ON`) |
| `test_*` | `build/tests/test_*` | Test executables |

---

## Verify Installation

```bash
# Check version
./build/cli/filevault version

# Run help
./build/cli/filevault help

# Quick encrypt/decrypt test
echo "Hello World" > test.txt
./build/cli/filevault encrypt test.txt
./build/cli/filevault decrypt test.txt.vault
cat test.txt.dec
```

---

## Troubleshooting

### OpenSSL not found

```
Could not find OpenSSL
```

**Fix**: Set `OPENSSL_ROOT_DIR`:
```bash
cmake -B build -DOPENSSL_ROOT_DIR=/path/to/openssl
```

### Qt 6 not found

```
Could not find Qt6
```

**Fix**: Set `Qt6_DIR` or `CMAKE_PREFIX_PATH`:
```bash
cmake -B build -DBUILD_GUI=ON -DQt6_DIR=/path/to/qt6/lib/cmake/Qt6
```

### Compiler warnings treated as errors

All warnings are treated as errors (`-Werror` / `/WX`). If you encounter third-party header warnings, you can disable this with:
```bash
cmake -B build -DCMAKE_C_FLAGS="-Wno-error"
```
