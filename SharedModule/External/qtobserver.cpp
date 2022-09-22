#include "qtobserver.h"
#include <QTimer>

#include <QFileInfo>
#include <QDateTime>

QtObserverData::QtObserverData()
    : m_doObserve([](const Observable*){})
{

}

void QtObserverData::observe()
{
    m_doObserve = [](const QtObserverData::Observable* observable){
        if(observable->Condition()) {
            observable->Handle();
        }
    };
    for(const QtObserverData::Observable* observable : m_observables) {
        m_doObserve(observable);
    }    
}

void QtObserverData::add(const FCondition& condition, const FHandle& handle)
{
    m_observables.Append(new QtObserverData::Observable{ condition, handle });
}

void QtObserverData::addFileObserver(const QString& file, const FHandle& handle)
{
    add([file, this]{
        QFileInfo fi(file);
        if(fi.exists()) {
            qint64 currentLastModified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(&file, currentLastModified);
        }
        return false;
    }, handle);
}

void QtObserverData::addFileObserver(const QString& dir, const QString& file, const FHandle& handle)
{
    add([dir,file,this]{
        QFileInfo fi(dir, file);
        if(fi.exists()) {
            qint64 currentLastModified = fi.lastModified().toMSecsSinceEpoch();
            return testValue(&file, currentLastModified);
        }
        return false;
    }, handle);
}

bool QtObserverData::testValue(const void* value, qint64 asInt64)
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

void QtObserverData::clear()
{
    m_doObserve = [](const QtObserverData::Observable*){};
    m_counters.clear();
    m_observables.Clear();
}


QtObserver::QtObserver(qint32 msInterval, const ThreadHandlerNoThreadCheck& threadHandler, QObject* parent)
    : QObject(parent)
    , d(::make_shared<QtObserverData>())
    , m_timer(msInterval)
{
    auto data = d;
    m_timer.OnTimeout([data, threadHandler]{
        threadHandler([data]{
            data->observe();
        });
    });
}

QtObserver::~QtObserver()
{

}
