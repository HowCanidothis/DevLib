#include "externalservice.h"


ExternalService::ExternalService(const QString& path)
    : m_restartAttempts(0)
    , m_maxRestartAttempts(3)
    , m_runningIntervalAfterError(10000)
    , m_path(path)
    , m_dying(false)
{
    connect(this, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [this](int , QProcess::ExitStatus status){
        if(m_dying) {
            return;
        }
        if(status == QProcess::CrashExit) {
            OnCrashed();
            printDebug("crashed");
        } else {
            OnExited();
            printDebug("exited");
        }
    });

    connect(this, &QProcess::errorOccurred, [this](QProcess::ProcessError error){
        if(!m_dying) {
            OnError(error);
            printDebug(errorString());
        }
    });

    connect(this, &QProcess::started, [this]{
        OnStarted();
        printDebug("started");
        ThreadTimer::SingleShot(2000, [this]{
            doAsync([this]{
                if(state() == Running) {
                    m_restartAttempts = 0;
                }
            });
        });
    });
}

ExternalService::ExternalService(const QString& path, ExternalService::Mode)
    : ExternalService(path)
{
    auto restartApp = [this, path]{
        start();
    };

    OnExited += { this, restartApp };
    OnError += { this, [this](ExternalService::ProcessError){
        ThreadTimer::SingleShot(m_runningIntervalAfterError, [this]{
            if(state() == NotRunning) {
                printDebug("restarted after error");
                doAsync([this]{
                    start();
                });
            }
        });
    }};
}

ExternalService::~ExternalService()
{
    m_dying = true;
    stop();
}

void ExternalService::Start()
{
    doAsync([this]{ start(); });
}

void ExternalService::Stop()
{
    doAsync([this]{ stop(); });
}

void ExternalService::stop()
{
    if(state() == Running) {
        terminate();
        if(!waitForFinished(3000)) {
            kill();
        }
    }
}

void ExternalService::doAsync(const FAction& handler)
{
    ThreadsBase::DoQThreadWorker(this, handler);
}

void ExternalService::start()
{
    if(state() == Running || state() == Starting) {
        return;
    }
    setWorkingDirectory(QDir::currentPath());
    if(m_restartAttempts < m_maxRestartAttempts) {
        m_restartAttempts++;
        Super::start(m_path);
    } else {
        OnRestartAttemptsExceed();
    }
}

void ExternalService::printDebug(const QString& message)
{
    qDebug() << QString("Process %1 %2").arg(m_path, message);
}
