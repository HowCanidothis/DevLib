#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QMutex>

class ProcessManager
{
private:
    friend class ProcessValue;

    static ProcessManager& getInstance();
    qint32 registerNewProcessValue();
    void unregisterProcessValue(qint32 depth);

    QMutex m_mutex;
    Array<qint32> m_registeredValues;
};

#endif // PROCESSMANAGER_H
