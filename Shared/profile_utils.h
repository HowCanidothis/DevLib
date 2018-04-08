#ifndef PROFILE_UTILS_H
#define PROFILE_UTILS_H

#include "shared_decl.h"

class QTextStream;

struct Timer{
    typedef qint64 nsecs;
    typedef qint32 msecs;

    Timer(){ bind(); }
    nsecs time;
    void bind(){ time = now(); }
    nsecs release()
    {
        nsecs res = now() - time;
        this->bind();
        return res;
    }
    static msecs toMsecs(nsecs nanosecs);

    static void print(const QString& lbl, qint64 time);
    static void print(QTextStream& stream, const QString& lbl, qint64 time);
    static QString text(const QString& lbl, qint64 time);
    QString text(const QString& lbl) const { return text(lbl, now() - time); }
    static qint64 now();
};

class TimerEnum
{
    Q_GADGET
public:
enum Timers{
    Apply,
    Count
};
Q_ENUM(Timers)
};


#if !defined(QT_NO_DEBUG) || defined(PROFILE_BUILD)
#define TIMER Timer local_timer
#define TIMER_RESTART(message) Timer::print(#message, local_timer.release())

class Timers
{
    struct SubTimer{
        Timer timer;
        qint64 total=0;
    };
    static SubTimer*& current(){ static SubTimer* res; return res; }
    static SubTimer* timers() { static SubTimer res[TimerEnum::Count]; return res; }
public:
    static void bind(qint32 j);
    static void add(qint32 j);
    static void set(qint32 j);
    static void reset(qint32 j);
    static qint64 get(qint32 j);

    static void reset();
    static void print();
    static void print(QTextStream& stream);
};

template<TimerEnum::Timers v>
struct TimerFunction
{
    TimerFunction(){ Timers::bind(v); }
    ~TimerFunction(){ Timers::set(v); }
};

template<TimerEnum::Timers v>
struct TimerFunctionAdder
{
    TimerFunctionAdder(){ Timers::bind(v); }
    ~TimerFunctionAdder(){ Timers::add(v); }
};

#define PROFILE_FUNCTION_ADD(Enum) TimerFunctionAdder<Enum> local_ftimer
#define PROFILE_FUNCTION_SET(Enum) TimerFunction<Enum> local_ftimer

#else
class Timers
{
public:
    static void bind(qint32){}
    static void add(qint32){}
    static void set(qint32){}
    static void reset(qint32){}
    static qint64 get(qint32) { return 0; }

    static void reset(){}
    static void print(){}
    static void print(QTextStream&){}
};

#define PROFILE_FUNCTION_ADD(Enum)
#define PROFILE_FUNCTION_SET(Enum)

#endif

struct FPSCounter
{
#define FRAMES_COUNT 30
    ScopedPointer<Timer> timer;
    qint64 last_30_frames_time[FRAMES_COUNT];
    qint32 current_frame_id;

    FPSCounter();

    void add(qint64 frame_time);
    void bind();
    qint64 release();

    double findMeanFPS() const;
};

#endif // PROFILE_UTILS_H
