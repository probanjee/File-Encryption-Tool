/* FileVault v2.0 — Qt GUI — Main Window */

#include "MainWindow.h"
#include "EncryptionWorker.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QSettings>
#include <QFileInfo>

extern "C" {
#include "filevault/version.h"
#include "filevault/password.h"
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
{
    setWindowTitle(QString("%1 v%2").arg(FV_APP_NAME, FV_VERSION_STRING));
    setMinimumSize(640, 720);
    resize(720, 780);
    setAcceptDrops(true);

    m_themeManager = new ThemeManager(this);

    setupUI();
    setupMenuBar();

    m_themeManager->loadSavedTheme();

    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait(3000);
    }
}

void MainWindow::setupMenuBar() {
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Settings...", this, &MainWindow::openSettings);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", qApp, &QApplication::quit);

    auto *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("Toggle &Theme", m_themeManager, &ThemeManager::toggleTheme);

    auto *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About FileVault", this, &MainWindow::openAbout);
}

void MainWindow::setupUI() {
    auto *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(24, 16, 24, 16);
    mainLayout->setSpacing(12);

    /* ── Title ── */
    auto *titleLabel = new QLabel("🔒 FileVault");
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto *subtitleLabel = new QLabel("Secure File Encryption");
    subtitleLabel->setStyleSheet("font-size: 13px; color: #8888aa;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(8);

    /* ── File Selection ── */
    auto *fileGroup = new QGroupBox("File");
    auto *fileLayout = new QVBoxLayout(fileGroup);

    /* Input row */
    auto *inputRow = new QHBoxLayout();
    auto *inputLabel = new QLabel("Input:");
    inputLabel->setFixedWidth(65);
    m_inputEdit = new QLineEdit();
    m_inputEdit->setPlaceholderText("Drop a file here or click Browse...");
    m_browseInputBtn = new QPushButton("Browse...");
    m_browseInputBtn->setFixedWidth(90);
    inputRow->addWidget(inputLabel);
    inputRow->addWidget(m_inputEdit);
    inputRow->addWidget(m_browseInputBtn);
    fileLayout->addLayout(inputRow);

    /* Output row */
    auto *outputRow = new QHBoxLayout();
    auto *outputLabel = new QLabel("Output:");
    outputLabel->setFixedWidth(65);
    m_outputEdit = new QLineEdit();
    m_outputEdit->setPlaceholderText("Auto-generated (optional override)");
    m_browseOutputBtn = new QPushButton("Browse...");
    m_browseOutputBtn->setFixedWidth(90);
    outputRow->addWidget(outputLabel);
    outputRow->addWidget(m_outputEdit);
    outputRow->addWidget(m_browseOutputBtn);
    fileLayout->addLayout(outputRow);

    mainLayout->addWidget(fileGroup);

    /* ── Mode Selection ── */
    auto *modeGroup = new QGroupBox("Mode");
    auto *modeLayout = new QHBoxLayout(modeGroup);

    m_encryptRadio = new QRadioButton("Encrypt");
    m_decryptRadio = new QRadioButton("Decrypt");
    m_encryptRadio->setChecked(true);

    m_algorithmLabel = new QLabel("AES-256-GCM • PBKDF2-HMAC-SHA256");
    m_algorithmLabel->setStyleSheet("color: #6688cc; font-size: 11px;");

    modeLayout->addWidget(m_encryptRadio);
    modeLayout->addWidget(m_decryptRadio);
    modeLayout->addStretch();
    modeLayout->addWidget(m_algorithmLabel);

    mainLayout->addWidget(modeGroup);

    /* ── Password ── */
    auto *passGroup = new QGroupBox("Password");
    auto *passLayout = new QVBoxLayout(passGroup);

    auto *passRow = new QHBoxLayout();
    auto *passLabel = new QLabel("Password:");
    passLabel->setFixedWidth(65);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter encryption password");
    passRow->addWidget(passLabel);
    passRow->addWidget(m_passwordEdit);
    passLayout->addLayout(passRow);

    auto *confirmRow = new QHBoxLayout();
    m_confirmLabel = new QLabel("Confirm:");
    m_confirmLabel->setFixedWidth(65);
    m_confirmEdit = new QLineEdit();
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    m_confirmEdit->setPlaceholderText("Confirm password");
    confirmRow->addWidget(m_confirmLabel);
    confirmRow->addWidget(m_confirmEdit);
    passLayout->addLayout(confirmRow);

    mainLayout->addWidget(passGroup);

    /* ── Action Buttons ── */
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_encryptBtn = new QPushButton("🔐  Encrypt File");
    m_encryptBtn->setObjectName("encryptBtn");
    m_encryptBtn->setFixedSize(200, 48);

    m_decryptBtn = new QPushButton("🔓  Decrypt File");
    m_decryptBtn->setObjectName("decryptBtn");
    m_decryptBtn->setFixedSize(200, 48);
    m_decryptBtn->setVisible(false);

    btnLayout->addWidget(m_encryptBtn);
    btnLayout->addWidget(m_decryptBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    /* ── Progress ── */
    auto *progressGroup = new QGroupBox("Progress");
    auto *progressLayout = new QVBoxLayout(progressGroup);

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    progressLayout->addWidget(m_progressBar);

    auto *statsRow = new QHBoxLayout();
    m_speedLabel = new QLabel("Speed: ---");
    m_speedLabel->setStyleSheet("font-size: 11px; color: #8888aa;");
    m_etaLabel = new QLabel("ETA: ---");
    m_etaLabel->setStyleSheet("font-size: 11px; color: #8888aa;");
    statsRow->addWidget(m_speedLabel);
    statsRow->addStretch();
    statsRow->addWidget(m_etaLabel);
    progressLayout->addLayout(statsRow);

    mainLayout->addWidget(progressGroup);

    /* ── Log Output ── */
    auto *logGroup = new QGroupBox("Activity Log");
    auto *logLayout = new QVBoxLayout(logGroup);

    m_logOutput = new QTextEdit();
    m_logOutput->setReadOnly(true);
    m_logOutput->setFixedHeight(120);
    m_logOutput->setStyleSheet("font-family: 'Consolas', 'Courier New', monospace; font-size: 12px;");
    logLayout->addWidget(m_logOutput);

    mainLayout->addWidget(logGroup);

    /* ── Connections ── */
    connect(m_browseInputBtn, &QPushButton::clicked, this, &MainWindow::onBrowseInput);
    connect(m_browseOutputBtn, &QPushButton::clicked, this, &MainWindow::onBrowseOutput);
    connect(m_encryptBtn, &QPushButton::clicked, this, &MainWindow::onEncrypt);
    connect(m_decryptBtn, &QPushButton::clicked, this, &MainWindow::onDecrypt);
    connect(m_encryptRadio, &QRadioButton::toggled, this, &MainWindow::onModeChanged);
    connect(m_decryptRadio, &QRadioButton::toggled, this, &MainWindow::onModeChanged);

    /* When input file changes, auto-detect mode */
    connect(m_inputEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (text.endsWith(".vault", Qt::CaseInsensitive)) {
            m_decryptRadio->setChecked(true);
        }
    });
}

void MainWindow::onModeChanged() {
    bool isEncrypt = m_encryptRadio->isChecked();
    m_encryptBtn->setVisible(isEncrypt);
    m_decryptBtn->setVisible(!isEncrypt);
    m_confirmEdit->setVisible(isEncrypt);
    m_confirmLabel->setVisible(isEncrypt);
    m_passwordEdit->setPlaceholderText(
        isEncrypt ? "Enter encryption password" : "Enter decryption password");
}

void MainWindow::onBrowseInput() {
    QString file = QFileDialog::getOpenFileName(this, "Select Input File");
    if (!file.isEmpty()) {
        m_inputEdit->setText(file);
    }
}

void MainWindow::onBrowseOutput() {
    QString file = QFileDialog::getSaveFileName(this, "Select Output File");
    if (!file.isEmpty()) {
        m_outputEdit->setText(file);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString path = urls.first().toLocalFile();
        m_inputEdit->setText(path);
        appendLog("INFO", "File dropped: " + QFileInfo(path).fileName());
    }
}

void MainWindow::onEncrypt() {
    QString input = m_inputEdit->text().trimmed();
    QString output = m_outputEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString confirm = m_confirmEdit->text();

    if (input.isEmpty()) {
        QMessageBox::warning(this, "Missing Input", "Please select an input file.");
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "Missing Password", "Password cannot be empty.");
        return;
    }

    if (password.length() < FV_MIN_PASSWORD) {
        QMessageBox::warning(this, "Weak Password",
            QString("Password must be at least %1 characters.").arg(FV_MIN_PASSWORD));
        return;
    }

    if (password != confirm) {
        QMessageBox::warning(this, "Password Mismatch", "Passwords do not match.");
        return;
    }

    appendLog("INFO", "Starting encryption of " + QFileInfo(input).fileName());
    setOperationInProgress(true);

    /* Create worker and thread */
    m_workerThread = new QThread();
    m_worker = new EncryptionWorker();
    m_worker->moveToThread(m_workerThread);

    m_worker->setOperation(EncryptionWorker::Encrypt);
    m_worker->setInputPath(input);
    m_worker->setOutputPath(output);
    m_worker->setPassword(password);

    QSettings settings("FileVault", "FileVault");
    m_worker->setForceOverwrite(settings.value("forceOverwrite", false).toBool());

    connect(m_workerThread, &QThread::started, m_worker, &EncryptionWorker::process);
    connect(m_worker, &EncryptionWorker::progressChanged,
            this, &MainWindow::onProgressChanged);
    connect(m_worker, &EncryptionWorker::finished,
            this, &MainWindow::onOperationFinished);
    connect(m_worker, &EncryptionWorker::error,
            this, &MainWindow::onOperationError);
    connect(m_worker, &EncryptionWorker::finished,
            m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished,
            m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished,
            m_workerThread, &QObject::deleteLater);

    /* Clear password from UI immediately after starting */
    m_passwordEdit->clear();
    m_confirmEdit->clear();

    m_workerThread->start();
}

void MainWindow::onDecrypt() {
    QString input = m_inputEdit->text().trimmed();
    QString output = m_outputEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (input.isEmpty()) {
        QMessageBox::warning(this, "Missing Input", "Please select an input file.");
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "Missing Password", "Password cannot be empty.");
        return;
    }

    appendLog("INFO", "Starting decryption of " + QFileInfo(input).fileName());
    setOperationInProgress(true);

    m_workerThread = new QThread();
    m_worker = new EncryptionWorker();
    m_worker->moveToThread(m_workerThread);

    m_worker->setOperation(EncryptionWorker::Decrypt);
    m_worker->setInputPath(input);
    m_worker->setOutputPath(output);
    m_worker->setPassword(password);

    QSettings settings("FileVault", "FileVault");
    m_worker->setForceOverwrite(settings.value("forceOverwrite", false).toBool());

    connect(m_workerThread, &QThread::started, m_worker, &EncryptionWorker::process);
    connect(m_worker, &EncryptionWorker::progressChanged,
            this, &MainWindow::onProgressChanged);
    connect(m_worker, &EncryptionWorker::finished,
            this, &MainWindow::onOperationFinished);
    connect(m_worker, &EncryptionWorker::error,
            this, &MainWindow::onOperationError);
    connect(m_worker, &EncryptionWorker::finished,
            m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished,
            m_worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished,
            m_workerThread, &QObject::deleteLater);

    m_passwordEdit->clear();
    m_workerThread->start();
}

void MainWindow::onProgressChanged(int percent, const QString &speed,
                                   const QString &eta) {
    m_progressBar->setValue(percent);
    m_speedLabel->setText("Speed: " + speed);
    m_etaLabel->setText("ETA: " + eta);
}

void MainWindow::onOperationFinished(bool success, const QString &message,
                                     const QString &outputPath,
                                     double duration, quint64 bytesProcessed) {
    setOperationInProgress(false);

    if (success) {
        m_progressBar->setValue(100);
        appendLog("OK", message);

        if (!outputPath.isEmpty()) {
            appendLog("INFO", "Output: " + outputPath);
        }

        appendLog("INFO", QString("Size: %1 | Duration: %2s")
                  .arg(formatSize(bytesProcessed))
                  .arg(duration, 0, 'f', 2));

        statusBar()->showMessage("Operation completed successfully", 5000);
    } else {
        m_progressBar->setValue(0);
        appendLog("ERROR", message);
        statusBar()->showMessage("Operation failed", 5000);
    }
}

void MainWindow::onOperationError(const QString &errorMessage,
                                  const QString &errorAction) {
    appendLog("ERROR", errorMessage);
    if (!errorAction.isEmpty()) {
        appendLog("HINT", errorAction);
    }
}

void MainWindow::setOperationInProgress(bool inProgress) {
    m_encryptBtn->setEnabled(!inProgress);
    m_decryptBtn->setEnabled(!inProgress);
    m_browseInputBtn->setEnabled(!inProgress);
    m_browseOutputBtn->setEnabled(!inProgress);
    m_inputEdit->setEnabled(!inProgress);
    m_outputEdit->setEnabled(!inProgress);
    m_passwordEdit->setEnabled(!inProgress);
    m_confirmEdit->setEnabled(!inProgress);
    m_encryptRadio->setEnabled(!inProgress);
    m_decryptRadio->setEnabled(!inProgress);

    if (inProgress) {
        m_progressBar->setValue(0);
        m_speedLabel->setText("Speed: ---");
        m_etaLabel->setText("ETA: ---");
    }
}

void MainWindow::appendLog(const QString &prefix, const QString &message) {
    QString color;
    if (prefix == "OK") color = "#00d27a";
    else if (prefix == "ERROR") color = "#ff4757";
    else if (prefix == "WARN") color = "#ffa502";
    else if (prefix == "HINT") color = "#70a1ff";
    else color = "#8888aa";

    m_logOutput->append(
        QString("<span style='color:%1'>[%2]</span> %3")
        .arg(color, prefix, message.toHtmlEscaped()));
}

QString MainWindow::formatSize(quint64 bytes) {
    if (bytes < 1024ULL)
        return QString::number(bytes) + " B";
    if (bytes < 1024ULL * 1024)
        return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024ULL * 1024 * 1024)
        return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

void MainWindow::openSettings() {
    SettingsDialog dialog(m_themeManager, this);
    dialog.exec();
}

void MainWindow::openAbout() {
    AboutDialog dialog(this);
    dialog.exec();
}
