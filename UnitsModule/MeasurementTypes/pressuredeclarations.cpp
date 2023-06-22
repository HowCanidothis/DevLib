#include "pressuredeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace PressureUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Pascals, []{return QObject::tr("pascals");                      }, []{ return QObject::tr("Pa");        }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(Kilopascals, []{return QObject::tr("kilopascals");                  }, []{ return QObject::tr("kPa");       }, 1000.0);
    IMPLEMENT_MEASUREMENT_UNIT(NewtonPerSquareMeter, []{return QObject::tr("newton/sq.meter");              }, []{ return QObject::tr("N/m2");      }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(Bars, []{return QObject::tr("bars");                         }, []{ return QObject::tr("bars");      }, 100000.0);
    IMPLEMENT_MEASUREMENT_UNIT(Megapascals, []{return QObject::tr("megapascals");                  }, []{ return QObject::tr("MPa");       }, 1000000.0);
    IMPLEMENT_MEASUREMENT_UNIT(Atmospheres, []{return QObject::tr("atmospheres");                  }, []{ return QObject::tr("atm");       }, 101325.0);
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerSquareCentimeter, []{return QObject::tr("kilogram-forces/sq.centimeter");}, []{ return QObject::tr("kgf/cm2");   }, 98066.5);
    IMPLEMENT_MEASUREMENT_UNIT(KilogramPerSquareMeter, []{return QObject::tr("kilogram-forces/sq.meter");     }, []{ return QObject::tr("kgf/m2");    }, 9.80665);
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareInch, []{return QObject::tr("pounds-force/sq.inch");         }, []{ return QObject::tr("psi");       }, 6894.757);
    IMPLEMENT_MEASUREMENT_UNIT(KilopoundsPerSquareInch, []{return QObject::tr("kilopounds-force/sq.inch");     }, []{ return QObject::tr("ksi");       }, 6894757);
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPerSquareFeet, []{return QObject::tr("pounds-force/sq.feet");         }, []{ return QObject::tr("lbf/ft2");   }, 47.880172);
    IMPLEMENT_MEASUREMENT_UNIT(PoundsPer100SquareFeet, []{return QObject::tr("pounds-force/100 sq.feet");     }, []{ return QObject::tr("lbf/100ft2");}, 0.47880172);
}

IMPLEMENT_MEASUREMENT(Pressure, TR(MeasurementTr::tr("Pressure")))
IMPLEMENT_MEASUREMENT(MudPressure, TR(MeasurementTr::tr("Mud Pressure")))
IMPLEMENT_MEASUREMENT(YieldPoint, TR(MeasurementTr::tr("Yield Point")))
IMPLEMENT_MEASUREMENT(YieldStrength, TR(MeasurementTr::tr("Yield Strength")))
IMPLEMENT_MEASUREMENT(YoungModulus, TR(MeasurementTr::tr("Young Modulus")))
