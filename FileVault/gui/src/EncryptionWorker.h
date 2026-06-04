/* FileVault v2.0 — Qt GUI — Encryption Worker (background thread) */

#ifndef ENCRYPTIONWORKER_H
#define ENCRYPTIONWORKER_H

#include <QObject>
#include <QString>

class EncryptionWorker : public QObject {
    Q_OBJECT

public:
    enum Operation { Encrypt, Decrypt };
    Q_ENUM(Operation)

    explicit EncryptionWorker(QObject *parent = nullptr);

    void setOperation(Operation op);
    void setInputPath(const QString &path);
    void setOutputPath(const QString &path);
    void setPassword(const QString &password);
    void setForceOverwrite(bool force);

public slots:
    void process();

signals:
    void progressChanged(int percent, const QString &speed, const QString &eta);
    void finished(bool success, const QString &message, const QString &outputPath,
                  double durationSeconds, quint64 bytesProcessed);
    void error(const QString &errorMessage, const QString &errorAction);

private:
    Operation m_operation;
    QString m_inputPath;
    QString m_outputPath;
    QString m_password;
    bool m_forceOverwrite;

    static void progressCallback(quint64 bytesDone, quint64 bytesTotal,
                                 void *userData);
};

#endif /* ENCRYPTIONWORKER_H */
