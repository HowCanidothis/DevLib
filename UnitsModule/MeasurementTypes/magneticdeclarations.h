#ifndef MAGNETICDECLARATIONS_H
#define MAGNETICDECLARATIONS_H

#include "measurementunitmanager.h"

static const Name MEASUREMENT_FIELD_STRENGTH = "Magnetic Field";

namespace FieldStrengthUnits
{
    static const MeasurementUnit NanoTeslas("NanoTeslas", []{return QObject::tr("nanoTeslas");}, []{ return QObject::tr("nT"); }, 1.0);
};

#endif // MAGNETICDECLARATIONS_H
