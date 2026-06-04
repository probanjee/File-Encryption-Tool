/* FileVault v2.0 — Qt GUI — Theme Manager */

#include "ThemeManager.h"
#include <QApplication>
#include <QSettings>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), m_currentTheme(Dark) {}

void ThemeManager::applyTheme(Theme theme) {
    m_currentTheme = theme;
    saveTheme(theme);

    QString qss = (theme == Dark) ? darkStyleSheet() : lightStyleSheet();
    qApp->setStyleSheet(qss);

    emit themeChanged(theme);
}

ThemeManager::Theme ThemeManager::currentTheme() const {
    return m_currentTheme;
}

void ThemeManager::toggleTheme() {
    applyTheme(m_currentTheme == Dark ? Light : Dark);
}

void ThemeManager::loadSavedTheme() {
    QSettings settings("FileVault", "FileVault");
    int saved = settings.value("theme", 0).toInt();
    applyTheme(saved == 1 ? Light : Dark);
}

void ThemeManager::saveTheme(Theme theme) {
    QSettings settings("FileVault", "FileVault");
    settings.setValue("theme", theme == Light ? 1 : 0);
}

QString ThemeManager::darkStyleSheet() {
    return QStringLiteral(R"(
        * {
            font-family: 'Segoe UI', 'Inter', 'Roboto', sans-serif;
            font-size: 13px;
        }

        QMainWindow, QDialog {
            background-color: #1a1a2e;
            color: #eaeaea;
        }

        QWidget {
            background-color: #1a1a2e;
            color: #eaeaea;
        }

        QGroupBox {
            border: 1px solid #2a2a4e;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 20px;
            font-weight: bold;
            color: #c0c0e0;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 16px;
            padding: 0 8px;
        }

        QLabel {
            color: #d0d0e8;
        }

        QLineEdit {
            background-color: #16213e;
            border: 1px solid #2a2a4e;
            border-radius: 6px;
            padding: 8px 12px;
            color: #eaeaea;
            selection-background-color: #0f3460;
        }
        QLineEdit:focus {
            border: 1px solid #4a6fa5;
        }

        QPushButton {
            background-color: #0f3460;
            color: #eaeaea;
            border: none;
            border-radius: 6px;
            padding: 10px 24px;
            font-weight: bold;
            min-height: 20px;
        }
        QPushButton:hover {
            background-color: #1a4f8a;
        }
        QPushButton:pressed {
            background-color: #0a2540;
        }
        QPushButton:disabled {
            background-color: #2a2a4e;
            color: #666688;
        }

        QPushButton#encryptBtn {
            background-color: #0f3460;
            font-size: 15px;
            padding: 12px 32px;
        }
        QPushButton#encryptBtn:hover {
            background-color: #1a5f9a;
        }

        QPushButton#decryptBtn {
            background-color: #2d4a22;
            font-size: 15px;
            padding: 12px 32px;
        }
        QPushButton#decryptBtn:hover {
            background-color: #3d6a32;
        }

        QRadioButton {
            color: #d0d0e8;
            spacing: 8px;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
        }

        QProgressBar {
            border: 1px solid #2a2a4e;
            border-radius: 6px;
            text-align: center;
            background-color: #16213e;
            color: #eaeaea;
            min-height: 24px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #0f3460, stop:1 #1a6fa5);
            border-radius: 5px;
        }

        QTextEdit {
            background-color: #16213e;
            border: 1px solid #2a2a4e;
            border-radius: 6px;
            color: #eaeaea;
            padding: 8px;
        }

        QMenuBar {
            background-color: #12122a;
            color: #c0c0e0;
            border-bottom: 1px solid #2a2a4e;
        }
        QMenuBar::item:selected {
            background-color: #0f3460;
        }

        QMenu {
            background-color: #16213e;
            color: #eaeaea;
            border: 1px solid #2a2a4e;
        }
        QMenu::item:selected {
            background-color: #0f3460;
        }

        QStatusBar {
            background-color: #12122a;
            color: #8888aa;
            border-top: 1px solid #2a2a4e;
        }

        QComboBox {
            background-color: #16213e;
            border: 1px solid #2a2a4e;
            border-radius: 6px;
            padding: 6px 12px;
            color: #eaeaea;
        }
        QComboBox:hover {
            border: 1px solid #4a6fa5;
        }
        QComboBox QAbstractItemView {
            background-color: #16213e;
            color: #eaeaea;
            selection-background-color: #0f3460;
        }

        QCheckBox {
            color: #d0d0e8;
            spacing: 8px;
        }
    )");
}

QString ThemeManager::lightStyleSheet() {
    return QStringLiteral(R"(
        * {
            font-family: 'Segoe UI', 'Inter', 'Roboto', sans-serif;
            font-size: 13px;
        }

        QMainWindow, QDialog {
            background-color: #f0f2f5;
            color: #1e293b;
        }

        QWidget {
            background-color: #f0f2f5;
            color: #1e293b;
        }

        QGroupBox {
            border: 1px solid #d1d5db;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 20px;
            font-weight: bold;
            color: #374151;
            background-color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 16px;
            padding: 0 8px;
        }

        QLabel {
            color: #374151;
        }

        QLineEdit {
            background-color: #ffffff;
            border: 1px solid #d1d5db;
            border-radius: 6px;
            padding: 8px 12px;
            color: #1e293b;
            selection-background-color: #bfdbfe;
        }
        QLineEdit:focus {
            border: 1px solid #2563eb;
        }

        QPushButton {
            background-color: #2563eb;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 10px 24px;
            font-weight: bold;
            min-height: 20px;
        }
        QPushButton:hover {
            background-color: #1d4ed8;
        }
        QPushButton:pressed {
            background-color: #1e40af;
        }
        QPushButton:disabled {
            background-color: #d1d5db;
            color: #9ca3af;
        }

        QPushButton#encryptBtn {
            background-color: #2563eb;
            font-size: 15px;
            padding: 12px 32px;
        }
        QPushButton#encryptBtn:hover {
            background-color: #1d4ed8;
        }

        QPushButton#decryptBtn {
            background-color: #16a34a;
            font-size: 15px;
            padding: 12px 32px;
        }
        QPushButton#decryptBtn:hover {
            background-color: #15803d;
        }

        QRadioButton {
            color: #374151;
            spacing: 8px;
        }

        QProgressBar {
            border: 1px solid #d1d5db;
            border-radius: 6px;
            text-align: center;
            background-color: #e5e7eb;
            color: #1e293b;
            min-height: 24px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563eb, stop:1 #60a5fa);
            border-radius: 5px;
        }

        QTextEdit {
            background-color: #ffffff;
            border: 1px solid #d1d5db;
            border-radius: 6px;
            color: #1e293b;
            padding: 8px;
        }

        QMenuBar {
            background-color: #ffffff;
            color: #374151;
            border-bottom: 1px solid #e5e7eb;
        }
        QMenuBar::item:selected {
            background-color: #eff6ff;
        }

        QMenu {
            background-color: #ffffff;
            color: #1e293b;
            border: 1px solid #d1d5db;
        }
        QMenu::item:selected {
            background-color: #eff6ff;
        }

        QStatusBar {
            background-color: #ffffff;
            color: #6b7280;
            border-top: 1px solid #e5e7eb;
        }

        QComboBox {
            background-color: #ffffff;
            border: 1px solid #d1d5db;
            border-radius: 6px;
            padding: 6px 12px;
            color: #1e293b;
        }
        QComboBox:hover {
            border: 1px solid #2563eb;
        }

        QCheckBox {
            color: #374151;
            spacing: 8px;
        }
    )");
}
