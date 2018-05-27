#include "profile_utils.h"
#include <chrono>

#include "timer.h"
#include "stack.h"

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

PerformanceClocks::PerformanceClocks(const char* caption, const char* file, quint32 line)
    : _messager(caption, file, line)
{
    getPerfomanceClocksInstances().Append(this);
}

void PerformanceClocks::PrintReport()
{
    for(PerformanceClocks* perfomanceClock : getPerfomanceClocksInstances()) {
        perfomanceClock->printReport();
    }
}

void PerformanceClocks::printReport() const
{
    _messager.Info() << CalculateMinValue().ToString("Min")
                     << CalculateMeanValue().ToString("Mean")
                     << CalculateMaxValue().ToString("Max");
}

Stack<PerformanceClocks*>&PerformanceClocks::getPerfomanceClocksInstances()
{
    static Stack<PerformanceClocks*> clocks;
    return clocks;
}



double Nanosecs::TimesPerSecond() const
{
    return 1000000000.0 / _nsecs;
}

QString Nanosecs::ToString(const QString& caption) const
{
    return Timer::TimeToStringSecs(caption, _nsecs);
}
