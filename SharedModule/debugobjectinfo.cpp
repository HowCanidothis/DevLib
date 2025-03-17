#include "debugobjectinfo.h"

#include "dispatcher.h"

QMutex DebugObjectManager::m_mutex;

void DebugObjectManager::Create(const char* location, const void* object, const QString& name, DispatcherConnectionsSafe& safeConnections)
{
    QMutexLocker locker(&m_mutex);
    qDebug() << "Debug object" << name << "is registered by key" << QString::number(size_t(object), 16);
    create(location, object, name, safeConnections);
}

void DebugObjectManager::create(const char* location, const void* object, const QString& name, DispatcherConnectionsSafe& safeConnections)
{
    QVector<DispatcherConnectionSafePtr> result;
    auto keyObject = reinterpret_cast<size_t>(object);
    safeConnections += ::make_shared<DispatcherConnectionSafe>(DispatcherConnection([keyObject]{
        QMutexLocker locker(&m_mutex);
        auto foundIt = debugInfo().find(keyObject);
        if(foundIt != debugInfo().end()) {
            for(auto* child : ::make_const(foundIt.value().Children)) {
                debugInfo().remove(reinterpret_cast<size_t>(child));
            }
            debugInfo().erase(foundIt);
        }
    }));
    auto& value = debugInfo()[keyObject];
    value.Location = location;
    value.ObjectName = name;
    for(auto* child : ::make_const(value.Children)) {
        create(location, child, name, safeConnections);
    }
}

const QString PrintPattern("From %1: Created at: %2; ObjectName: %3");

void DebugObjectManager::PrintInfo(const char* location, const void* object, const FAction& onPrinted)
{
    QMutexLocker locker(&m_mutex);
    const auto& info = debugInfo();
    auto foundIt = info.find(reinterpret_cast<size_t>(object));
    if(foundIt != info.end() && !foundIt.value().ObjectName.isEmpty()) {
        qDebug() << PrintPattern.arg(location, foundIt.value().Location, foundIt.value().ObjectName);
        locker.unlock();
        if(onPrinted != nullptr) {
            onPrinted();
        }
    }
}

const DebugObjectManager::DebugInfo DebugObjectManager::GetInfo(const void* object)
{
    QMutexLocker locker(&m_mutex);
    const auto& info = debugInfo();
    auto foundIt = info.find(reinterpret_cast<size_t>(object));
    if(foundIt != info.end() && !foundIt.value().ObjectName.isEmpty()) {
        return foundIt.value();
    }
    return DebugInfo();
}

void DebugObjectManager::Synchronize(const void* object, const QSet<const void*>& children)
{
    QMutexLocker locker(&m_mutex);
    auto& value = debugInfo()[reinterpret_cast<size_t>(object)];
    Q_ASSERT(value.ObjectName.isEmpty());
    value.Children += children;
}

QHash<size_t, DebugObjectManager::DebugInfo>& DebugObjectManager::debugInfo()
{
    static QHash<size_t, DebugInfo> result;
    return result;
}
