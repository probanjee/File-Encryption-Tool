/* FileVault v2.0 — Qt GUI — Theme Manager */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>

class QApplication;

class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum Theme { Dark, Light };
    Q_ENUM(Theme)

    explicit ThemeManager(QObject *parent = nullptr);

    void applyTheme(Theme theme);
    Theme currentTheme() const;
    void toggleTheme();
    void loadSavedTheme();

    static QString darkStyleSheet();
    static QString lightStyleSheet();

signals:
    void themeChanged(Theme newTheme);

private:
    Theme m_currentTheme;
    void saveTheme(Theme theme);
};

#endif /* THEMEMANAGER_H */
