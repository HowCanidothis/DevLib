#include "magneticdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FieldStrengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(NanoTeslas, []{return QObject::tr("nanoTeslas");}, []{ return QObject::tr("nT"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(MicroTeslas, []{return QObject::tr("microTeslas");}, []{ return QObject::tr("muT"); }, 1000.0);
    IMPLEMENT_MEASUREMENT_UNIT(Gauss, []{return QObject::tr("gauss");}, []{ return QObject::tr("G"); }, 100000.0);
};

IMPLEMENT_MEASUREMENT(MagneticField, TR(MeasurementTr::tr("Magnetic Field")))
