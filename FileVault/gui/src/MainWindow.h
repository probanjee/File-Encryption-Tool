/* FileVault v2.0 — Qt GUI — Main Window */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "ThemeManager.h"

class QLineEdit;
class QPushButton;
class QRadioButton;
class QProgressBar;
class QLabel;
class QTextEdit;
class EncryptionWorker;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onBrowseInput();
    void onBrowseOutput();
    void onEncrypt();
    void onDecrypt();
    void onModeChanged();
    void onProgressChanged(int percent, const QString &speed, const QString &eta);
    void onOperationFinished(bool success, const QString &message,
                             const QString &outputPath, double duration,
                             quint64 bytesProcessed);
    void onOperationError(const QString &errorMessage, const QString &errorAction);
    void openSettings();
    void openAbout();

private:
    void setupUI();
    void setupMenuBar();
    void setOperationInProgress(bool inProgress);
    void appendLog(const QString &prefix, const QString &message);
    QString formatSize(quint64 bytes);

    /* UI widgets */
    QLineEdit    *m_inputEdit;
    QLineEdit    *m_outputEdit;
    QLineEdit    *m_passwordEdit;
    QLineEdit    *m_confirmEdit;
    QLabel       *m_confirmLabel;
    QRadioButton *m_encryptRadio;
    QRadioButton *m_decryptRadio;
    QPushButton  *m_encryptBtn;
    QPushButton  *m_decryptBtn;
    QPushButton  *m_browseInputBtn;
    QPushButton  *m_browseOutputBtn;
    QProgressBar *m_progressBar;
    QLabel       *m_speedLabel;
    QLabel       *m_etaLabel;
    QLabel       *m_algorithmLabel;
    QTextEdit    *m_logOutput;

    /* Worker thread */
    QThread          *m_workerThread;
    EncryptionWorker *m_worker;

    /* Theme manager */
    ThemeManager *m_themeManager;
};

#endif /* MAINWINDOW_H */
