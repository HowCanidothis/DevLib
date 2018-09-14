#include "qtobserver.h"
#include <QTimer>

#include <QFileInfo>
#include <QDateTime>

QtObserver::QtObserver(qint32 msInterval, QObject* parent)
    : QObject(parent)
    , _doObserve([](const Observable*){})
{
    auto timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(msInterval);
}

void QtObserver::Add(const FCondition& condition, const FHandle& handle)
{
    this->_observables.Append(new Observable{ condition, handle });
}

void QtObserver::AddFilePtrObserver(const QString* fileName, const QtObserver::FHandle& handle)
{
    Add([fileName, this]{
        QFileInfo fi(*fileName);
        if(fi.exists()) {
            qint64 currentLastModified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(fileName, currentLastModified);
        }
        return false;
    }, handle);
}

void QtObserver::AddFilePtrObserver(const QString* dir, const QString* file, const QtObserver::FHandle& handle)
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

void QtObserver::AddFileObserver(const QString& file, const FHandle& handle)
{
    Add([file, this]{
        QFileInfo fi(file);
        if(fi.exists()) {
            qint64 currentLastModified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(&file, currentLastModified);
        }
        return false;
    }, handle);
}

void QtObserver::AddFileObserver(const QString& dir, const QString& file, const FHandle& handle)
{
    Add([dir,file,this]{
        QFileInfo fi(file);
        DirBinder dbinder(dir);
        if(fi.exists()) {
            qint64 currentLastModified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(&file, currentLastModified);
        }
        return false;
    }, handle);
}

void QtObserver::AddFloatObserver(const float* value, const FHandle& handle)
{
    Add([value,this](){
        qint64 asInt64 = *(const qint32*)value;
        return testValue(value, asInt64);
    }, handle);
}

void QtObserver::AddStringObserver(const QString* value, const FHandle& handle)
{
    Add([value,this](){
        qint64 asInt64 = qHash(*value);
        return testValue(value, asInt64);
    }, handle);
}

void QtObserver::Clear()
{
    _doObserve = [](const Observable*){};
    _counters.clear();
    _observables.Clear();
}

void QtObserver::onTimeout()
{
    for(const Observable* observable : _observables) {
        _doObserve(observable);
    }
    _doObserve = [](const Observable* observable){
        if(observable->Condition()) {
            observable->Handle();
        }
    };
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
