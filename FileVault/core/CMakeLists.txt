# ──────────────────────────────────────────────────────────────────────
# FileVault Core Library
# Static library containing the encryption engine, file format,
# password handling, file utilities, legacy XOR, and secure memory.
# ──────────────────────────────────────────────────────────────────────

add_library(filevault_core STATIC
    src/errors.c
    src/secure_mem.c
    src/file_utils.c
    src/password.c
    src/file_format.c
    src/crypto_engine.c
    src/legacy_xor.c
)

target_include_directories(filevault_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(filevault_core PUBLIC
    OpenSSL::Crypto
)

# Platform-specific link libraries
if(WIN32)
    target_link_libraries(filevault_core PUBLIC ws2_32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(filevault_core PUBLIC m)
endif()
