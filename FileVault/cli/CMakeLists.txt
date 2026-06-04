# ──────────────────────────────────────────────────────────────────────
# FileVault CLI Application
# ──────────────────────────────────────────────────────────────────────

add_executable(filevault
    src/main.c
    src/cli_commands.c
    src/cli_progress.c
)

target_include_directories(filevault PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(filevault PRIVATE
    filevault_core
)

# Install target
install(TARGETS filevault RUNTIME DESTINATION bin)
