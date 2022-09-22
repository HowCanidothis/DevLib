#ifndef FILEOBSERVER_H
#define FILEOBSERVER_H

#include <QObject>
#include <functional>

#include "SharedModule/internal.hpp"

class QtObserverData
{
    typedef std::function<void ()> FHandle;
    typedef std::function<bool ()> FCondition;

    struct Observable
    {
        FCondition Condition;
        FHandle Handle;
    };

    typedef std::function<void (const Observable*)> FObserve;

    ArrayPointers<Observable> m_observables;
    QHash<const void*, qint64> m_counters;
    FObserve m_doObserve;
    friend class QtObserver;
public:
    QtObserverData();

private:
    void observe();
    void add(const FCondition& condition, const FHandle& handle);
    void addFileObserver(const QString& file, const FHandle& handle);
    void addFileObserver(const QString& dir, const QString& file, const FHandle& handle);
    bool testValue(const void* value, qint64 asInt64);
    void clear();
};

class QtObserver : public QObject
{
    Q_OBJECT
    typedef std::function<void ()> FHandle;
    typedef std::function<bool ()> FCondition;

    SharedPointer<QtObserverData> d;
    ThreadTimer m_timer;

public:
    QtObserver(qint32 msInterval, const ThreadHandlerNoThreadCheck& threadHandler, QObject* parent=0);
    ~QtObserver();

    void Add(const FCondition& condition, const FHandle& handle) { d->add(condition, handle); }
    void AddFileObserver(const QString& file, const FHandle& handle) { d->addFileObserver(file, handle); }
    void AddFileObserver(const QString& dir, const QString& file, const FHandle& handle) { d->addFileObserver(dir, file, handle); }
    void Clear() { d->clear(); }

    void Observe() { d->observe(); }

    static QtObserver* Instance() { static QtObserver* res = new QtObserver(1000, ThreadHandlerNoCheckMainLowPriority); return res; }
};

#endif // FILEOBSERVER_H
