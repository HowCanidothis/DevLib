#include "timer.h"
#include <chrono>

using namespace std;
using chrono::duration_cast;
using chrono::duration;
using chrono::nanoseconds;
typedef duration<float,micro> microseconds;
typedef duration<float,milli> milliseconds;
typedef duration<float> seconds;
typedef duration<float,ratio<60>> minutes;
typedef duration<float,ratio<3600>> hours;

Timer::Timer()
{

}

void Timer::Bind()
{
    _time = Now();
}

Timer::nsecs Timer::Release()
{
    nsecs res = Now() - _time;
    this->Bind();
    return res;
}

Timer::msecs Timer::ToMsecs(nsecs nanosecs)
{
    return duration_cast<milliseconds>(nanoseconds(nanosecs)).count();
}

qint64 Timer::Now()
{
    return duration_cast<nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); //nanosecs
}

QString Timer::TimeToStringSecs(const QString& lbl, qint64 time)
{
    const qint64 thousand = 1000;
    const qint64 million = thousand * thousand;
    const qint64 billion = million * thousand;

    return lbl + " " + QString::number(double(time) / billion, 'f', 9) + " secs";;
}
