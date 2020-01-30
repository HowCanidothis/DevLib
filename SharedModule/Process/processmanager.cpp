#include "processmanager.h"

ProcessManager& ProcessManager::getInstance()
{
    static ProcessManager result;
    return result;
}

qint32 ProcessManager::registerNewProcessValue()
{
    QMutexLocker locker(&m_mutex);
    auto value = m_registeredValues.IsEmpty() ? 0 : (m_registeredValues.Last() + 1);
    m_registeredValues.Append(value);
    return value;
}

void ProcessManager::unregisterProcessValue(qint32 depth)
{
    QMutexLocker locker(&m_mutex);
    auto depthToRemove = m_registeredValues.FindSorted(depth);
    m_registeredValues.Remove(depthToRemove);
}

