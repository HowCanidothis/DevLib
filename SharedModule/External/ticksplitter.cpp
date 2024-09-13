#include "ticksplitter.h"

#include <qmath.h>

TickSplitter::TickSplitter(qint32 prefferedTicksCount)
    : m_prefferedTicksCount(prefferedTicksCount)
{

}

float TickSplitter::TicksStepForRange(float range) const
{
    float exactStep = range/((float)m_prefferedTicksCount + 1e-6f); // mTickCount ticks on average, the small addition is to prevent jitter on exact integers
    return cleanMantissa(exactStep);
}

double TickSplitter::cleanMantissa(double input) const
{
    double magnitude;
    const double mantissa = getMantissa(input, &magnitude);
    return pickClosest(mantissa, QVector<double>() << 1.0 << 2.0 << 2.5 << 5.0 << 10.0)*magnitude;
}

double TickSplitter::getMantissa(double input, double *magnitude) const
{
    const double mag = qPow(10.0, qFloor(qLn(input)/qLn(10.0)));
    if (magnitude) *magnitude = mag;
    return input/mag;
}

QVector<double> TickSplitter::CreateTickVector(float from, float to, float tickStep) const
{
    QVector<double> result;
    if(to < from) {
        tickStep = -tickStep;
    }
    // Generate tick positions according to tickStep:
    qint64 firstStep = floor((from)/tickStep); // do not use qFloor here, or we'll lose 64 bit precision
    qint64 lastStep = ceil((to)/tickStep); // do not use qCeil here, or we'll lose 64 bit precision
    int tickcount = abs(lastStep - firstStep) + 1;

    result.resize(tickcount);
    for (int i=0; i < tickcount; ++i) {
        result[i] = (firstStep+i) * tickStep;
    }
    return result;
}

double TickSplitter::pickClosest(double target, const QVector<double> &candidates) const
{
    if (candidates.size() == 1)
        return candidates.first();
    QVector<double>::const_iterator it = std::lower_bound(candidates.constBegin(), candidates.constEnd(), target);
    if (it == candidates.constEnd())
        return *(it-1);
    else if (it == candidates.constBegin())
        return *it;
    else
        return target-*(it-1) < *it-target ? *(it-1) : *it;
}
