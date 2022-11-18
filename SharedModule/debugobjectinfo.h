#ifndef DEBUGOBJECTINFO_H
#define DEBUGOBJECTINFO_H

#include <QHash>
#include <QString>

#define DEBUG_DETAILS

#ifdef QT_DEBUG
#define DEBUG_LOCATION __FILE__ QT_STRINGIFY(__LINE__)
#else
#define DEBUG_LOCATION nullptr
#endif

#if defined(QT_DEBUG) && defined(DEBUG_DETAILS)
#define DEBUG_CREATE_INFO(x, name, connections) DebugObjectManager::Create(DEBUG_LOCATION, x, name, connections);
#define DEBUG_PRINT_INFO(x) DebugObjectManager::PrintInfo(DEBUG_LOCATION, x)
#define DEBUG_PRINT_INFO_ACTION(x, action) DebugObjectManager::PrintInfo(DEBUG_LOCATION, x, [&]{ action })
#define DEBUG_SYNC(x, ...) DebugObjectManager::Synchronize(x, __VA_ARGS__);
#else
#define DEBUG_CREATE_INFO(x, name, connections)
#define DEBUG_PRINT_INFO(x)
#define DEBUG_PRINT_INFO_ACTION(x, action)
#define DEBUG_SYNC(x, ...)
#endif

class DebugObjectManager
{
    DebugObjectManager() {}
public:
    struct DebugInfo
    {
        const char* Location;
        QString ObjectName;
        QSet<void*> Children;
    };

    static void Create(const char* location, void* object, const QString& name, QVector<SharedPointer<class DispatcherConnectionSafe>>& connections);
    static void PrintInfo(const char* location, void* object, const std::function<void ()>& onPrinted = nullptr);
    static void Synchronize(void* object, const QSet<void*>& children);
    static const DebugInfo GetInfo(void* object);

private:
    static void create(const char* location, void* object, const QString& name, QVector<SharedPointer<DispatcherConnectionSafe>>& connections);
    static QHash<size_t, DebugInfo>& debugInfo();
    static QMutex m_mutex;
};

#endif // DEBUGOBJECTINFO_H
