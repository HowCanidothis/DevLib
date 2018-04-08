#ifndef FILEOBSERVER_H
#define FILEOBSERVER_H

#include <QObject>
#include <functional>

#include <QVector>
#include "SharedGui/gt_decl.h"

class QtObserver : public QObject
{
    Q_OBJECT
    typedef std::function<void ()> Handle;
    typedef std::function<bool ()> Condition;

    QVector<Condition> conditions;
    QVector<Handle> handles;
    QHash<const void*, qint64> counters;
public:
    QtObserver(qint32 ms_interval, QObject* parent=0);

    void add(const Condition& condition, const Handle& handle);
    void addFileObserver(const QString* file, const Handle& handle);
    void addFileObserver(const QString* dir, const QString* file, const Handle& handle);
    void addFloatObserver(const float* value, const Handle& handle);
    void addStringObserver(const QString* value, const Handle& handle);

    void observe() { onTimeout(); }

    static QtObserver* instance() { static QtObserver* res = new QtObserver(1000); return res; }
private Q_SLOTS:
    void onTimeout();

private:
    bool testValue(const void* value, qint64 as_int64);
};

#endif // FILEOBSERVER_H
