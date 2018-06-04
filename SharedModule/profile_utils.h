#ifndef PROFILE_UTILS_H
#define PROFILE_UTILS_H

#include "decl.h"

#ifndef CLOCKS_COUNT
#define CLOCKS_COUNT 30
#endif

class Timer;

class Nanosecs
{
public:
    Nanosecs(double nsecs)
        : _nsecs(nsecs)
    {}

    double TimesPerSecond() const;
    QString ToString(const QString& caption) const;

    operator double() const { return _nsecs; }

private:
    double _nsecs;
};

class TimerClocks
{
public:
    class Guard
    {
        TimerClocks* _timerClocks;
    public:
        Guard(TimerClocks* timerClocks)
            : _timerClocks(timerClocks)
        {
            _timerClocks->Bind();
        }
        ~Guard()
        {
            _timerClocks->Release();
        }
    };

    TimerClocks();
    ~TimerClocks();

    Guard Clock()
    {
        return Guard(this);
    }

    void Bind();
    qint64 Release();

    Nanosecs CalculateMeanValue() const;
    Nanosecs CalculateMinValue() const;
    Nanosecs CalculateMaxValue() const;

private:
    ScopedPointer<Timer> _timer;
    qint64 _clocks[CLOCKS_COUNT];
    qint32 _currentClockIndex;
};

template<class T, template<typename> class Ptr> class Stack;

class PerformanceClocks : public TimerClocks
{
    Messager _messager;
public:
    PerformanceClocks(const char* caption, const char* file, quint32 line);

    static void PrintReport();

private:
    void printReport() const;
    static Stack<PerformanceClocks*>& getPerfomanceClocksInstances();
};

#if !defined(QT_NO_DEBUG) || defined(PROFILE_BUILD)
#define __PERFOMANCE__ \
    static PerformanceClocks pClock##__LINE__(__FUNCTION__, __FILE__, __LINE__); \
    pClock##__LINE__.Clock();
#else
#define __PERFOMANCE__
#endif

#endif // PROFILE_UTILS_H
