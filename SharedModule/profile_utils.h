#ifndef PROFILE_UTILS_H
#define PROFILE_UTILS_H

#include "stack.h"
#include "smartpointersadapters.h"
#include "shared_decl.h"

#ifndef CLOCKS_COUNT
#define CLOCKS_COUNT 30
#endif

class Timer;

class _Export TimerClocks
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
    virtual ~TimerClocks();

    Guard Clock()
    {
        return Guard(this);
    }

    void Bind();
    virtual qint64 Release();
    Nanosecs ReleaseNanosecs();

    Nanosecs CalculateMeanValue() const;
    Nanosecs CalculateMinValue() const;
    Nanosecs CalculateMaxValue() const;

    void ToTextStream(QTextStream& stream) const;
    virtual QString ToString() const;

protected:
    ScopedPointer<Timer> _timer;
    qint64 _clocks[CLOCKS_COUNT];
    qint32 _currentClockIndex;
};

template<class T, template<typename> class Ptr> class Stack;

class _Export PerformanceClocks : public TimerClocks
{
    using Super = TimerClocks;
    const char* _function;
    const char* _file;
    quint32 _line;
    qint64 m_totalTime;
public:
    PerformanceClocks(const char* function, const char* file, quint32 line);

    qint64 Release() override;
    QString ToString() const override;

    static void PrintReport();

private:
    void printReport() const;
    static Stack<PerformanceClocks*>& getPerfomanceClocksInstances();
};

#define COMBINE1(X,Y) X##Y  // helper macro
#define COMBINE(X,Y) COMBINE1(X,Y)
#define COMBINE_3(X,Y,Z) COMBINE(COMBINE(X,Y), Z)

#ifdef SHOW_HIDDEN_FUNCTIONALITY
#define __PERFORMANCE__ \
    static PerformanceClocks COMBINE(pClock,__LINE__)(__FUNCTION__, __FILE__, __LINE__); \
    auto COMBINE_3(pClock,__LINE__,guard) = COMBINE(pClock,__LINE__).Clock()
#else
#define __PERFORMANCE__
#endif

#endif // PROFILE_UTILS_H
