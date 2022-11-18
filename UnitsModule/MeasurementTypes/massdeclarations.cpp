#include "massdeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace MassUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Kilograms, []{return QObject::tr("kilograms"); }, []{ return QObject::tr("kg");  }, 1.0  );
    IMPLEMENT_MEASUREMENT_UNIT(Grams, []{return QObject::tr("grams");     }, []{ return QObject::tr("g");   }, 0.001  );
    IMPLEMENT_MEASUREMENT_UNIT(Tonnes, []{return QObject::tr("tonnes");    }, []{ return QObject::tr("t");   }, 1000.0 );
    IMPLEMENT_MEASUREMENT_UNIT(Pounds, []{return QObject::tr("pounds");    }, []{ return QObject::tr("lbs"); }, 0.453592);
    IMPLEMENT_MEASUREMENT_UNIT(Kilopounds, []{return QObject::tr("kilopounds");}, []{ return QObject::tr("klbs");}, 453.592);
}

IMPLEMENT_MEASUREMENT(Mass, TR(MeasurementTr::tr("Mass")))
