#include "concentrationdeclaration.h"

#include "UnitsModule/measurementunitmanager.h"

namespace ConcentrationUnits {
    IMPLEMENT_MEASUREMENT_UNIT(MilliliterPerMilliliter, []{return QObject::tr("milliliter/milliliter");}, []{ return QObject::tr("mL/mL"); }, 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(PartsPerBillion, []{return QObject::tr("Parts Per Billion");}, []{ return QObject::tr("ppb"); }, 0.0028530101742118)
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerCubicMeters, []{return QObject::tr("m3/m3");}, []{ return QObject::tr("m3/m3"); }, 1.)
    IMPLEMENT_MEASUREMENT_UNIT(KilogrammPerCubicMeters, []{return QObject::tr("kg/m3");}, []{ return QObject::tr("kg/m3"); }, 0.001)
    IMPLEMENT_MEASUREMENT_UNIT(MilligrammPerLiter, []{return QObject::tr("milligramm per liter");}, []{ return QObject::tr("mg/L"); }, 0.000001)
    IMPLEMENT_MEASUREMENT_UNIT(GallonPerMegagallon, []{return QObject::tr("Gallon Per Megagallon");}, []{ return QObject::tr("gal/Mgal"); }, 0.000001)
}

IMPLEMENT_MEASUREMENT(Concentration, TR(MeasurementTr::tr("Concentration")))
IMPLEMENT_MEASUREMENT(ConcentrationMud, TR(MeasurementTr::tr("ConcentrationMud")))
IMPLEMENT_MEASUREMENT(ConcentrationAlkalinity, TR(MeasurementTr::tr("Concentration Alkalinity")))
