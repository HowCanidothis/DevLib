#ifndef MASSDECLARATIONS_H
#define MASSDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_MASS = "Mass";

namespace MassUnits
{
    static const MeasurementUnit Kilograms  ("Kilograms ", []{return QObject::tr("kilograms"); }, []{ return QObject::tr("kg");  }, 2.2046226  );
    static const MeasurementUnit Grams      ("Grams     ", []{return QObject::tr("grams");     }, []{ return QObject::tr("g");   }, 0.0022046  );
    static const MeasurementUnit Tonnes     ("Tonnes    ", []{return QObject::tr("tonnes");    }, []{ return QObject::tr("t");   }, 2204.622622);
    static const MeasurementUnit Pounds     ("Pounds    ", []{return QObject::tr("pounds");    }, []{ return QObject::tr("lbs"); }, 1          );
	static const MeasurementUnit Kilopounds ("Kilopounds", []{return QObject::tr("kilopounds");}, []{ return QObject::tr("klbf");}, 1000       );
}

#define MEASUREMENT_MASS_STRING              MEASUREMENT_STRING(MEASUREMENT_MASS)
#define MEASUREMENT_MASS_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_MASS)
#define MEASUREMENT_MASS_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_MASS, x)
#define MEASUREMENT_MASS_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_MASS, x)
#define MEASUREMENT_MASS_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_MASS, x)

#endif // MASSDECLARATIONS_H
