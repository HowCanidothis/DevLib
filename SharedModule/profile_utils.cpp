#include "profile_utils.h"
#include <chrono>

#include "timer.h"
#include "internal.hpp"

TimerClocks::TimerClocks()
    : _timer(new Timer)
    , _clocks {0,}
    , _currentClockIndex(0)
{

}

TimerClocks::~TimerClocks()
{

}

void TimerClocks::Bind()
{
    _timer->Bind();
}

qint64 TimerClocks::Release()
{
    auto nsecs = _timer->Release();
    _clocks[_currentClockIndex % CLOCKS_COUNT] = nsecs;
    ++_currentClockIndex;
    return nsecs;
}

Nanosecs TimerClocks::ReleaseNanosecs()
{
    return Release();
}

Nanosecs TimerClocks::CalculateMeanValue() const
{
    qint64 sum = 0;
    for(qint64 time : _clocks) {
        sum += time;
    }
    return double(sum) / CLOCKS_COUNT;
}

Nanosecs TimerClocks::CalculateMinValue() const
{
    qint64 min = INT64_MAX;
    for(qint64 time : _clocks) {
        if(time < min) {
            min = time;
        }
    }
    return min;
}

Nanosecs TimerClocks::CalculateMaxValue() const
{
    qint64 max = INT64_MIN;
    for(qint64 time : _clocks) {
        if(time > max) {
            max = time;
        }
    }
    return max;
}

void TimerClocks::ToTextStream(QTextStream& stream) const
{
    stream << CalculateMinValue().ToString("Min")
           << CalculateMeanValue().ToString(" Mean")
           << CalculateMaxValue().ToString(" Max");
}

QString TimerClocks::ToString() const
{
    QString result;
    QTextStream stream(&result);
    ToTextStream(stream);
    return result;
}

PerformanceClocks::PerformanceClocks(const char* function, const char* file, quint32 line)
    : _function(function)
    , _file(file)
    , _line(line)
    , m_totalTime(0)
{
    getPerfomanceClocksInstances().Append(this);
}

qint64 PerformanceClocks::Release()
{
    m_totalTime += _timer->ElapsedTime();
    return Super::Release();
}

QString PerformanceClocks::ToString() const
{
    auto result = Super::ToString();
    result += Nanosecs(m_totalTime).ToString(" Total Time");
    return result;
}

void PerformanceClocks::PrintReport()
{
    for(PerformanceClocks* perfomanceClock : getPerfomanceClocksInstances()) {
        perfomanceClock->printReport();
    }
}

void PerformanceClocks::printReport() const
{
    QMessageLogger(_file, _line, nullptr).debug("ASSERT: %s in file %s, line %d", ToString().toLatin1().data(), QString(_file).toLatin1().data(), _line);
}

Stack<PerformanceClocks*>&PerformanceClocks::getPerfomanceClocksInstances()
{
    static Stack<PerformanceClocks*> clocks;
    return clocks;
}
