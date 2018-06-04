#ifndef FILEOBSERVER_H
#define FILEOBSERVER_H

#include <QObject>
#include <functional>

#include <QVector>
#include "SharedGuiModule/decl.h"

class QtObserver : public QObject
{
    Q_OBJECT
    typedef std::function<void ()> Handle;
    typedef std::function<bool ()> Condition;

    QVector<Condition> _conditions;
    QVector<Handle> _handles;
    QHash<const void*, qint64> _counters;
public:
    QtObserver(qint32 msInterval, QObject* parent=0);

    void Add(const Condition& condition, const Handle& handle);
    void AddFileObserver(const QString* file, const Handle& handle);
    void AddFileObserver(const QString* dir, const QString* file, const Handle& handle);
    void AddFloatObserver(const float* value, const Handle& handle);
    void AddStringObserver(const QString* value, const Handle& handle);

    void Observe() { onTimeout(); }

    static QtObserver* Instance() { static QtObserver* res = new QtObserver(1000); return res; }
private Q_SLOTS:
    void onTimeout();

private:
    bool testValue(const void* value, qint64 asInt64);
};

#endif // FILEOBSERVER_H
