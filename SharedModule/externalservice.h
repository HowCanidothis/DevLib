#ifndef EXTERNALSERVICE_H
#define EXTERNALSERVICE_H

#include <QProcess>

#include "dispatcher.h"

class ExternalService : public QProcess
{
    using Super = QProcess;
public:
    enum Mode {
        Mode_KeepRunning
    };

    ExternalService(const QString& path);
    ExternalService(const QString& path, Mode);

    ~ExternalService();

    void SetRunningIntervalAfterError(qint32 msecs) { m_runningIntervalAfterError = msecs; }
    void Start();
    void Stop();
    void SetMaxRestartShots(qint32 maxRestartShots) { m_maxRestartAttempts = maxRestartShots; }

    Dispatcher OnStarted;
    Dispatcher OnCrashed;
    Dispatcher OnExited;
    CommonDispatcher<ExternalService::ProcessError> OnError;
    Dispatcher OnRestartAttemptsExceed;

private:
    void stop();
    void doAsync(const FAction& handler);
    void start();

    void printDebug(const QString& message);

private:
    qint32 m_restartAttempts;
    qint32 m_maxRestartAttempts;
    qint32 m_runningIntervalAfterError;
    QString m_path;
    bool m_dying;
};

#endif // EXTERNALSERVICE_H
