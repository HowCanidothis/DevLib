#include "volumeperlengthdeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace VolumePerLengthUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(CubicMeterPerMeter, []{return QObject::tr("cubic meter per meter");}, []{ return QObject::tr("m3/m"); }, 1);
    IMPLEMENT_MEASUREMENT_UNIT(BarrelPerFoot, []{return QObject::tr("barrel per foot");}, []{ return QObject::tr("bbl/ft"); }, 0.52161187664042);
    IMPLEMENT_MEASUREMENT_UNIT(LiterPerMeter, []{return QObject::tr("liter per meter");}, []{ return QObject::tr("L/m"); }, 0.001);
};

IMPLEMENT_MEASUREMENT(VolumePerLength)
IMPLEMENT_MEASUREMENT(LinearCapacity)
