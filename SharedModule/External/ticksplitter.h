#ifndef TICKSPLITTER_H
#define TICKSPLITTER_H


class TickSplitter
{
public:
    TickSplitter(qint32 prefferedTicksCount = 5);

    QVector<double> CreateTickVector(float from, float to, float step) const;
    float TicksStepForRange(float range) const;

private:
    double cleanMantissa(double input) const;
    double pickClosest(double target, const QVector<double> &candidates) const;
    double getMantissa(double input, double *magnitude) const;

private:
    qint32 m_prefferedTicksCount;
};

#endif // TICKSPLITTER_H
