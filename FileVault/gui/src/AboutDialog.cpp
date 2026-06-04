/* FileVault v2.0 — Qt GUI — About Dialog */

#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

extern "C" {
#include "filevault/version.h"
#include "filevault/password.h"
}

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About FileVault");
    setFixedSize(420, 380);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 32, 32, 24);
    layout->setSpacing(12);

    /* Title */
    auto *titleLabel = new QLabel(QStringLiteral("🔒 FileVault"));
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    /* Version */
    auto *versionLabel = new QLabel(
        QString("Version %1").arg(FV_VERSION_STRING));
    versionLabel->setStyleSheet("font-size: 14px; color: #8888aa;");
    versionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(versionLabel);

    layout->addSpacing(12);

    /* Description */
    auto *descLabel = new QLabel(
        "Secure file encryption tool using industry-standard\n"
        "AES-256-GCM authenticated encryption.\n\n"
        "Key Derivation: PBKDF2-HMAC-SHA256\n"
        QString("Iterations: %1\n").arg(FV_PBKDF2_ITERS) +
        "Random salt and nonce per file.\n"
        "No passwords or keys are ever stored.");
    descLabel->setWordWrap(true);
    descLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(descLabel);

    layout->addSpacing(12);

    /* Copyright */
    auto *copyrightLabel = new QLabel(FV_COPYRIGHT);
    copyrightLabel->setStyleSheet("font-size: 11px; color: #666688;");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(copyrightLabel);

    auto *licenseLabel = new QLabel("Licensed under the MIT License");
    licenseLabel->setStyleSheet("font-size: 11px; color: #666688;");
    licenseLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(licenseLabel);

    layout->addStretch();

    /* Close button */
    auto *closeBtn = new QPushButton("Close");
    closeBtn->setFixedWidth(100);
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
