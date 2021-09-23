#ifndef PRESSUREDECLARATIONS_H
#define PRESSUREDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_PRESSURE = "Pressure";

namespace PressureUnits
{
    static const MeasurementUnit Pascals                     ("Pascals                       ", []{return QObject::tr("pascals");                      }, []{ return QObject::tr("Pa");        }, 0.000145        );
    static const MeasurementUnit Kilopascals                 ("Kilopascals                   ", []{return QObject::tr("kilopascals");                  }, []{ return QObject::tr("kPa");       }, 0.1450377       );
    static const MeasurementUnit Bars                        ("Bars                          ", []{return QObject::tr("bars");                         }, []{ return QObject::tr("bars");      }, 14.5037738      );
    static const MeasurementUnit Megapascals                 ("Megapascals                   ", []{return QObject::tr("megapascals");                  }, []{ return QObject::tr("MPa");       }, 145.0377377     );
	static const MeasurementUnit Atmospheres                 ("Atmospheres                   ", []{return QObject::tr("atmospheres");                  }, []{ return QObject::tr("atm");       }, 14.6959488      );
	static const MeasurementUnit KilogramPerSquareCentimeter ("Kilogram Per Square Centimeter", []{return QObject::tr("kilogram-forces/sq.centimeter");}, []{ return QObject::tr("kgf/cm2");   }, 14.2233433      );
	static const MeasurementUnit PoundsPerSquareInch         ("Pounds Per Square Inch        ", []{return QObject::tr("pounds-force/sq.inch");         }, []{ return QObject::tr("psi");       }, 1               );
	static const MeasurementUnit KilopoundsPerSquareInch     ("Kilopounds Per Square Inch    ", []{return QObject::tr("kilopounds-force/sq.inch");     }, []{ return QObject::tr("ksi");       }, 1000            );
	static const MeasurementUnit PoundsPerSquareFeet         ("Pounds Per 100 Square Feet    ", []{return QObject::tr("pounds-force/100 sq.feet");     }, []{ return QObject::tr("lbs/100ft2");}, 0.00006944444444);
}

#define MEASUREMENT_PRESSURE_STRING              MEASUREMENT_STRING(MEASUREMENT_PRESSURE)
#define MEASUREMENT_PRESSURE_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_PRESSURE)
#define MEASUREMENT_PRESSURE_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_PRESSURE, x)
#define MEASUREMENT_PRESSURE_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_PRESSURE, x)
#define MEASUREMENT_PRESSURE_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_PRESSURE, x)
#define ATTACH_PRESSURE_MEASUREMENT(delegate, min, max, step) ATTACH_MEASUREMENT(MEASUREMENT_PRESSURE, delegate, min, max, step)
#endif // PRESSUREDECLARATIONS_H
