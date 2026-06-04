# ──────────────────────────────────────────────────────────────────────
# FileVault Test Suite
# ──────────────────────────────────────────────────────────────────────

# Each test is a separate executable
set(TEST_SOURCES
    test_roundtrip.c
    test_wrong_password.c
    test_tampered.c
    test_empty.c
    test_legacy_xor.c
    test_large_file.c
)

foreach(test_source ${TEST_SOURCES})
    # Extract test name from filename (e.g., test_roundtrip.c → test_roundtrip)
    get_filename_component(test_name ${test_source} NAME_WE)

    add_executable(${test_name} ${test_source})

    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(${test_name} PRIVATE
        filevault_core
    )

    # Register with CTest
    add_test(
        NAME ${test_name}
        COMMAND ${test_name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()
