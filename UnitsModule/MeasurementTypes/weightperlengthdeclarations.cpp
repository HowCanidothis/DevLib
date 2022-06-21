#include "weightperlengthdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace WeightPerLengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerMeter, []{return QObject::tr("kilogram per meter");}, []{ return QObject::tr("kg/m"); }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerCantimeter, []{return QObject::tr("kilogram per cantimeter");}, []{ return QObject::tr("kg/cm"); }, 100.0);
    IMPLEMENT_MEASUREMENT_UNIT(PoundPerFoot, []{return QObject::tr("pound per foot");}, []{ return QObject::tr("lb/ft"); }, 1.48816);
    IMPLEMENT_MEASUREMENT_UNIT(PoundPerInch, []{return QObject::tr("pound per inch");}, []{ return QObject::tr("lb/in"); }, 17.88);
};

IMPLEMENT_MEASUREMENT(WeightPerLength)
