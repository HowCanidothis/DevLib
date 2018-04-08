#include "qtobserver.h"
#include <QTimer>

#include <QFileInfo>
#include <QDateTime>

QtObserver::QtObserver(qint32 ms_interval, QObject* parent)
    : QObject(parent)
{
    auto timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(ms_interval);
}

void QtObserver::add(const QtObserver::Condition& condition, const QtObserver::Handle& handle)
{
    this->conditions.append(condition);
    this->handles.append(handle);
}

void QtObserver::addFileObserver(const QString* file, const QtObserver::Handle& handle)
{
    add([file,this](){
        QFileInfo fi(*file);
        if(fi.exists()) {
            qint64 current_last_modified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(file, current_last_modified);
        }
        return false;
    }, handle);
}

void QtObserver::addFileObserver(const QString* dir, const QString* file, const QtObserver::Handle& handle)
{
    add([dir,file,this]{
        QFileInfo fi(*file);
        DirBinder dbinder(*dir);
        if(fi.exists()) {
            qint64 current_last_modified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(file, current_last_modified);
        }
        return false;
    }, handle);
}

void QtObserver::addFloatObserver(const float* value, const QtObserver::Handle& handle)
{
    add([value,this](){
        qint64 as_int64 = *(const qint32*)value;
        return testValue(value, as_int64);
    }, handle);
}

void QtObserver::addStringObserver(const QString* value, const QtObserver::Handle& handle)
{
    add([value,this](){
        qint64 as_int64 = qHash(*value);
        return testValue(value, as_int64);
    }, handle);
}

void QtObserver::onTimeout()
{
    auto it = handles.begin();
    for(const Condition&  c: conditions) {
        if(c()) {
            (*it)();
        }
        it++;
    }
}

bool QtObserver::testValue(const void* value, qint64 as_int64)
{
    auto find = counters.find(value);
    if(find == counters.end()) {
        counters.insert(value, as_int64);
    } else if(find.value() != as_int64){
        find.value() = as_int64;
        return true;
    }
    return false;
}
