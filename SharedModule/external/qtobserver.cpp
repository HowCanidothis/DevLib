#include "qtobserver.h"
#include <QTimer>

#include <QFileInfo>
#include <QDateTime>

QtObserver::QtObserver(qint32 msInterval, QObject* parent)
    : QObject(parent)
{
    auto timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(msInterval);
}

void QtObserver::Add(const QtObserver::Condition& condition, const QtObserver::Handle& handle)
{
    this->_conditions.append(condition);
    this->_handles.append(handle);
}

void QtObserver::AddFileObserver(const QString* file, const QtObserver::Handle& handle)
{
    Add([file,this] {
        QFileInfo fi(*file);
        if(fi.exists()) {
            qint64 currentLastModified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(file, currentLastModified);
        }
        return false;
    }, handle);
}

void QtObserver::AddFileObserver(const QString* dir, const QString* file, const QtObserver::Handle& handle)
{
    Add([dir,file,this]{
        QFileInfo fi(*file);
        DirBinder dbinder(*dir);
        if(fi.exists()) {
            qint64 currentLastModified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(file, currentLastModified);
        }
        return false;
    }, handle);
}

void QtObserver::AddFloatObserver(const float* value, const QtObserver::Handle& handle)
{
    Add([value,this](){
        qint64 asInt64 = *(const qint32*)value;
        return testValue(value, asInt64);
    }, handle);
}

void QtObserver::AddStringObserver(const QString* value, const QtObserver::Handle& handle)
{
    Add([value,this](){
        qint64 asInt64 = qHash(*value);
        return testValue(value, asInt64);
    }, handle);
}

void QtObserver::onTimeout()
{
    auto it = _handles.begin();
    for(const Condition&  c: _conditions) {
        if(c()) {
            (*it)();
        }
        it++;
    }
}

bool QtObserver::testValue(const void* value, qint64 asInt64)
{
    auto find = _counters.find(value);
    if(find == _counters.end()) {
        _counters.insert(value, asInt64);
    } else if(find.value() != asInt64){
        find.value() = asInt64;
        return true;
    }
    return false;
}
