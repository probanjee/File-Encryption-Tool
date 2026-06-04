cmake_minimum_required(VERSION 3.20)

project(FileVault
    VERSION     2.0.0
    DESCRIPTION "Secure File Encryption Tool"
    LANGUAGES   C CXX
)

# ──────────────────────────────────────────────────────────────────────
# Global settings
# ──────────────────────────────────────────────────────────────────────
set(CMAKE_C_STANDARD   17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Strict warnings
if(MSVC)
    add_compile_options(/W4 /WX /D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# ──────────────────────────────────────────────────────────────────────
# Options
# ──────────────────────────────────────────────────────────────────────
option(BUILD_GUI   "Build the Qt 6 GUI application"  OFF)
option(BUILD_TESTS "Build the test suite"             ON)

# ──────────────────────────────────────────────────────────────────────
# Dependencies
# ──────────────────────────────────────────────────────────────────────
find_package(OpenSSL REQUIRED)

# ──────────────────────────────────────────────────────────────────────
# Subdirectories
# ──────────────────────────────────────────────────────────────────────
add_subdirectory(core)
add_subdirectory(cli)

if(BUILD_GUI)
    add_subdirectory(gui)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
