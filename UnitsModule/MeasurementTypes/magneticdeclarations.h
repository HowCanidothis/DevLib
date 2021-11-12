#ifndef MAGNETICDECLARATIONS_H
#define MAGNETICDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_FIELD_STRENGTH = "Magnetic Field";

namespace FieldStrengthUnits
{
    static const MeasurementUnit NanoTeslas("NanoTeslas", []{return QObject::tr("nanoTeslas");}, []{ return QObject::tr("nT"); }, 1.0);
    static const MeasurementUnit MicroTeslas("MicroTeslas", []{return QObject::tr("microTeslas");}, []{ return QObject::tr("muT"); }, 1000.0);
    static const MeasurementUnit Gauss("Gauss", []{return QObject::tr("gauss");}, []{ return QObject::tr("G"); }, 100000.0);
};

#endif // MAGNETICDECLARATIONS_H
