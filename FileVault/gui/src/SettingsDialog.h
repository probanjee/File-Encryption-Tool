/* FileVault v2.0 — Qt GUI — Settings Dialog */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ThemeManager.h"

class QComboBox;
class QCheckBox;
class QLineEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(ThemeManager *themeManager,
                            QWidget *parent = nullptr);

    bool forceOverwrite() const;
    QString defaultOutputDir() const;

signals:
    void settingsChanged();

private slots:
    void onThemeChanged(int index);
    void onBrowseOutputDir();
    void onSave();

private:
    ThemeManager *m_themeManager;
    QComboBox    *m_themeCombo;
    QCheckBox    *m_forceOverwriteCheck;
    QLineEdit    *m_outputDirEdit;

    void loadSettings();
    void saveSettings();
};

#endif /* SETTINGSDIALOG_H */
