/* FileVault v2.0 — Qt GUI — Settings Dialog */

#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>

SettingsDialog::SettingsDialog(ThemeManager *themeManager, QWidget *parent)
    : QDialog(parent)
    , m_themeManager(themeManager)
{
    setWindowTitle("Settings");
    setFixedSize(480, 320);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 16);
    mainLayout->setSpacing(16);

    /* Appearance group */
    auto *appearGroup = new QGroupBox("Appearance");
    auto *appearLayout = new QFormLayout(appearGroup);

    m_themeCombo = new QComboBox();
    m_themeCombo->addItem("Dark Theme");
    m_themeCombo->addItem("Light Theme");
    appearLayout->addRow("Theme:", m_themeCombo);

    mainLayout->addWidget(appearGroup);

    /* File handling group */
    auto *fileGroup = new QGroupBox("File Handling");
    auto *fileLayout = new QFormLayout(fileGroup);

    m_forceOverwriteCheck = new QCheckBox("Overwrite existing output files");
    fileLayout->addRow("", m_forceOverwriteCheck);

    auto *outputRow = new QHBoxLayout();
    m_outputDirEdit = new QLineEdit();
    m_outputDirEdit->setPlaceholderText("Same as input file (default)");
    auto *browseBtn = new QPushButton("Browse...");
    browseBtn->setFixedWidth(90);
    outputRow->addWidget(m_outputDirEdit);
    outputRow->addWidget(browseBtn);
    fileLayout->addRow("Default output:", outputRow);

    mainLayout->addWidget(fileGroup);

    /* Buttons */
    mainLayout->addStretch();

    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setFixedWidth(100);
    btnLayout->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton("Save");
    saveBtn->setFixedWidth(100);
    btnLayout->addWidget(saveBtn);

    mainLayout->addLayout(btnLayout);

    /* Connections */
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onThemeChanged);
    connect(browseBtn, &QPushButton::clicked,
            this, &SettingsDialog::onBrowseOutputDir);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);

    loadSettings();
}

void SettingsDialog::loadSettings() {
    QSettings settings("FileVault", "FileVault");

    int theme = settings.value("theme", 0).toInt();
    m_themeCombo->setCurrentIndex(theme);

    m_forceOverwriteCheck->setChecked(
        settings.value("forceOverwrite", false).toBool());
    m_outputDirEdit->setText(
        settings.value("defaultOutputDir", "").toString());
}

void SettingsDialog::saveSettings() {
    QSettings settings("FileVault", "FileVault");
    settings.setValue("forceOverwrite", m_forceOverwriteCheck->isChecked());
    settings.setValue("defaultOutputDir", m_outputDirEdit->text());
}

void SettingsDialog::onThemeChanged(int index) {
    if (m_themeManager) {
        m_themeManager->applyTheme(
            index == 1 ? ThemeManager::Light : ThemeManager::Dark);
    }
}

void SettingsDialog::onBrowseOutputDir() {
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Default Output Directory",
        m_outputDirEdit->text());
    if (!dir.isEmpty()) {
        m_outputDirEdit->setText(dir);
    }
}

void SettingsDialog::onSave() {
    saveSettings();
    emit settingsChanged();
    accept();
}

bool SettingsDialog::forceOverwrite() const {
    return m_forceOverwriteCheck->isChecked();
}

QString SettingsDialog::defaultOutputDir() const {
    return m_outputDirEdit->text();
}
