#include "profile_utils.h"
#include <chrono>
#include <QDebug>

using namespace std;
using chrono::duration_cast;
using chrono::duration;
using chrono::nanoseconds;
typedef duration<float,micro> microseconds;
typedef duration<float,milli> milliseconds;
typedef duration<float> seconds;
typedef duration<float,ratio<60>> minutes;
typedef duration<float,ratio<3600>> hours;


Timer::msecs Timer::toMsecs(nsecs nanosecs)
{
    return duration_cast<milliseconds>(nanoseconds(nanosecs)).count();
}

void Timer::print(const QString& lbl, qint64 time){
    static const qint64 thousand = 1000;
    static const qint64 million = thousand * thousand;
    static const qint64 billion = million * thousand;
    static const qint64 billion60 = billion * 60;
    static const qint64 billion60x60 = billion60 * 60;

    if(time < billion60)
        qDebug() << lbl << QString::number(double(time) / billion, 'f', 9) << "secs";
    else if(time < billion60x60)
        qDebug() << lbl << double(time) / billion60 << "mins";
    else
        qDebug() << lbl << double(time) / billion60x60 << "hours";
}

qint64 Timer::now()
{
    return duration_cast<nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); //nanosecs
}

void Timer::print(QTextStream& stream, const QString& lbl, qint64 time)
{
    const qint64 thousand = 1000;
    const qint64 million = thousand * thousand;
    const qint64 billion = million * thousand;
    const qint64 billion60 = billion * 60;
    const qint64 billion60x60 = billion60 * 60;

    if(time < billion60)
        stream << lbl << " " << QString::number(double(time) / billion, 'f', 9) << " secs";
    else if(time < billion60x60)
       stream << lbl << " " << double(time) / billion60 << " mins";
    else
        stream << lbl << " " << double(time) / billion60x60 << " hours";
}

QString Timer::text(const QString& lbl, qint64 time)
{
    QString res;
    QTextStream stream(&res);
    print(stream, lbl, time);
    return res;
}

#if !defined(QT_NO_DEBUG) || defined(PROFILE_BUILD)

void Timers::bind(qint32 j)
{
    timers()[j].timer.bind();
}

void Timers::add(qint32 j)
{
    SubTimer& sub_timer = timers()[j];
    sub_timer.total += sub_timer.timer.release();
}

void Timers::set(qint32 j)
{
    SubTimer& sub_timer = timers()[j];
    sub_timer.total = sub_timer.timer.release();
}

qint64 Timers::get(qint32 j)
{
    SubTimer& sub_timer = timers()[j];
    return sub_timer.total;
}

void Timers::reset(qint32 j)
{
    SubTimer& sub_timer = timers()[j];
    sub_timer.total = 0;
}

void Timers::reset()
{
    for(qint32 i(0); i < TimerEnum::Count; i++)
        timers()[i].total = 0;
}

void Timers::print(){
    for(qint32 i(0); i < TimerEnum::Count; i++)
        Timer::print(QString(QMetaEnum::fromType<TimerEnum::Timers>().valueToKey(i)) + ": ", timers()[i].total);
}

void Timers::print(QTextStream& stream)
{
    for(qint32 i(0); i < TimerEnum::Count; i++)
        Timer::print(stream, QString(QMetaEnum::fromType<TimerEnum::Timers>().valueToKey(i)) + ": ", timers()[i].total);
}

#endif

FPSCounter::FPSCounter()
    : timer(new Timer)
#ifdef HOME
    , last_30_frames_time {0}
#endif
    , current_frame_id(0)
{

}

void FPSCounter::bind()
{
    timer->bind();
}

qint64 FPSCounter::release()
{
    qint64 res = timer->release();
    add(res);
    return res;
}

double FPSCounter::findMeanFPS() const
{
    qint64 sum = 0;
    for(qint64 time : last_30_frames_time)
        sum += time;
    double mean_frame_time = double(sum) / FRAMES_COUNT;
    return 1000000000.0 / mean_frame_time;
}

void FPSCounter::add(qint64 frame_time)
{
    last_30_frames_time[current_frame_id++] = frame_time;
    if(current_frame_id == FRAMES_COUNT) {
        current_frame_id = 0;
    }
}
