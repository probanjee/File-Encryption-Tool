/* FileVault v2.0 — Qt GUI — Encryption Worker */

#include "EncryptionWorker.h"

#include <QElapsedTimer>

extern "C" {
#include "filevault/crypto_engine.h"
#include "filevault/password.h"
#include "filevault/errors.h"
#include "filevault/secure_mem.h"
}

struct WorkerCallbackData {
    EncryptionWorker *worker;
    QElapsedTimer    timer;
};

EncryptionWorker::EncryptionWorker(QObject *parent)
    : QObject(parent)
    , m_operation(Encrypt)
    , m_forceOverwrite(false) {}

void EncryptionWorker::setOperation(Operation op)      { m_operation = op; }
void EncryptionWorker::setInputPath(const QString &p)   { m_inputPath = p; }
void EncryptionWorker::setOutputPath(const QString &p)  { m_outputPath = p; }
void EncryptionWorker::setPassword(const QString &p)    { m_password = p; }
void EncryptionWorker::setForceOverwrite(bool f)        { m_forceOverwrite = f; }

void EncryptionWorker::progressCallback(quint64 bytesDone, quint64 bytesTotal,
                                        void *userData) {
    auto *data = static_cast<WorkerCallbackData *>(userData);
    if (!data || !data->worker || bytesTotal == 0) return;

    int percent = static_cast<int>((bytesDone * 100ULL) / bytesTotal);

    double elapsed = data->timer.elapsed() / 1000.0;
    QString speed = QStringLiteral("---");
    QString eta = QStringLiteral("---");

    if (elapsed > 0.1) {
        double rate = static_cast<double>(bytesDone) / elapsed;
        if (rate > 1024.0 * 1024.0 * 1024.0) {
            speed = QString::number(rate / (1024.0*1024.0*1024.0), 'f', 2) + " GB/s";
        } else if (rate > 1024.0 * 1024.0) {
            speed = QString::number(rate / (1024.0*1024.0), 'f', 1) + " MB/s";
        } else if (rate > 1024.0) {
            speed = QString::number(rate / 1024.0, 'f', 1) + " KB/s";
        } else {
            speed = QString::number(rate, 'f', 0) + " B/s";
        }

        if (bytesDone < bytesTotal && rate > 0) {
            double remaining = static_cast<double>(bytesTotal - bytesDone) / rate;
            if (remaining < 60.0) {
                eta = QString::number(remaining, 'f', 0) + "s";
            } else {
                eta = QString::number(remaining / 60.0, 'f', 0) + "m";
            }
        } else if (bytesDone >= bytesTotal) {
            eta = QStringLiteral("done");
        }
    }

    emit data->worker->progressChanged(percent, speed, eta);
}

void EncryptionWorker::process() {
    QByteArray inputUtf8  = m_inputPath.toUtf8();
    QByteArray outputUtf8 = m_outputPath.toUtf8();
    QByteArray passUtf8   = m_password.toUtf8();

    WorkerCallbackData cbData;
    cbData.worker = this;
    cbData.timer.start();

    FvResult result;
    FvError err;

    if (m_operation == Encrypt) {
        FvEncryptRequest req;
        memset(&req, 0, sizeof(req));
        req.input_path      = inputUtf8.constData();
        req.output_path     = outputUtf8.isEmpty() ? nullptr : outputUtf8.constData();
        req.password        = passUtf8.constData();
        req.force_overwrite = m_forceOverwrite ? 1 : 0;
        req.progress_fn     = EncryptionWorker::progressCallback;
        req.progress_data   = &cbData;

        err = fv_encrypt_file(&req, &result);
    } else {
        FvDecryptRequest req;
        memset(&req, 0, sizeof(req));
        req.input_path      = inputUtf8.constData();
        req.output_path     = outputUtf8.isEmpty() ? nullptr : outputUtf8.constData();
        req.password        = passUtf8.constData();
        req.force_overwrite = m_forceOverwrite ? 1 : 0;
        req.verify_only     = 0;
        req.progress_fn     = EncryptionWorker::progressCallback;
        req.progress_data   = &cbData;

        err = fv_decrypt_file(&req, &result);
    }

    /* Securely wipe the password copy */
    fv_secure_zero(passUtf8.data(), static_cast<size_t>(passUtf8.size()));

    if (err == FV_OK) {
        QString mode = (m_operation == Encrypt) ? "Encryption" : "Decryption";
        QString msg = mode + " completed successfully.";
        emit finished(true, msg, QString::fromUtf8(result.output_path),
                      result.duration_seconds, result.bytes_processed);
    } else {
        emit error(QString::fromUtf8(fv_error_message(err)),
                   QString::fromUtf8(fv_error_action(err)));
        emit finished(false, QString::fromUtf8(fv_error_message(err)),
                      QString(), 0.0, 0);
    }
}
