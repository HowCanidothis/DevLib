#ifndef FILEOBSERVER_H
#define FILEOBSERVER_H

#include <QObject>
#include <functional>

#include "SharedModule/internal.hpp"

class QtObserver : public QObject
{
    Q_OBJECT
    typedef std::function<void ()> FHandle;
    typedef std::function<bool ()> FCondition;

    struct Observable
    {
        FCondition Condition;
        FHandle Handle;
    };

    typedef std::function<void (const Observable*)> FObserve;

    ArrayPointers<Observable> _observables;
    QHash<const void*, qint64> _counters;
    FObserve _doObserve;
public:
    QtObserver(qint32 msInterval, QObject* parent=0);

    void Add(const FCondition& condition, const FHandle& handle);
    void AddFilePtrObserver(const QString* fileName, const FHandle& handle);
    void AddFilePtrObserver(const QString* dir, const QString* file, const FHandle& handle);
    void AddFileObserver(const QString& file, const FHandle& handle);
    void AddFileObserver(const QString& dir, const QString& file, const FHandle& handle);
    void AddFloatObserver(const float* value, const FHandle& handle); // Where to use? Marked as deprecated. Use Properties Rx system instead
    void AddStringObserver(const QString* value, const FHandle& handle); // Where to use? Marked as deprecated. Use Properties Rx system instead
    void Clear();

    void Observe() { onTimeout(); }

    static QtObserver* Instance() { static QtObserver* res = new QtObserver(1000); return res; }
private Q_SLOTS:
    void onTimeout();

private:
    bool testValue(const void* value, qint64 asInt64);
};

#endif // FILEOBSERVER_H
