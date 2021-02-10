#include "qtobserver.h"
#include <QTimer>

#include <QFileInfo>
#include <QDateTime>

QtObserver::QtObserver(qint32 msInterval, const ThreadHandlerNoThreadCheck& threadHandler, QObject* parent)
    : QObject(parent)
    , m_timer(msInterval)
    , m_doObserve([](const Observable*){})
{
    m_timer.OnTimeout([this, threadHandler]{
        threadHandler([this]{
            onTimeout();
        });
    });
}

QtObserver::~QtObserver()
{

}

void QtObserver::Add(const FCondition& condition, const FHandle& handle)
{
    this->m_observables.Append(new Observable{ condition, handle });
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
        QFileInfo fi(*dir, *file);
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
        QFileInfo fi(dir, file);
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
    m_doObserve = [](const Observable*){};
    m_counters.clear();
    m_observables.Clear();
}

void QtObserver::onTimeout()
{
    for(const Observable* observable : m_observables) {
        m_doObserve(observable);
    }
    m_doObserve = [](const Observable* observable){
        if(observable->Condition()) {
            observable->Handle();
        }
    };
}

bool QtObserver::testValue(const void* value, qint64 asInt64)
{
    auto find = m_counters.find(value);
    if(find == m_counters.end()) {
        m_counters.insert(value, asInt64);
    } else if(find.value() != asInt64){
        find.value() = asInt64;
        return true;
    }
    return false;
}
