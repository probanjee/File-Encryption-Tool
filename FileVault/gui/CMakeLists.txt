# ──────────────────────────────────────────────────────────────────────
# FileVault Qt 6 GUI Application
# ──────────────────────────────────────────────────────────────────────

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(filevault-gui
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/EncryptionWorker.cpp
    src/EncryptionWorker.h
    src/SettingsDialog.cpp
    src/SettingsDialog.h
    src/AboutDialog.cpp
    src/AboutDialog.h
    src/ThemeManager.cpp
    src/ThemeManager.h
)

target_include_directories(filevault-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(filevault-gui PRIVATE
    filevault_core
    Qt6::Widgets
)

# Windows: hide console window in release builds
set_target_properties(filevault-gui PROPERTIES
    WIN32_EXECUTABLE $<CONFIG:Release>
)

install(TARGETS filevault-gui RUNTIME DESTINATION bin)
