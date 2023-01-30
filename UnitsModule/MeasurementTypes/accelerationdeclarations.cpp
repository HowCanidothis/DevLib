#include "accelerationdeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace AccelerationUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerSqSec,      []{return QObject::tr("Meters Per Square Second");},    []{ return QObject::tr("m/s2");      },     1.0);
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerSqMinute,   []{return QObject::tr("Meters Per Square Minute");},    []{ return QObject::tr("m/min2");      },   3600.0);
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerSqHour,     []{return QObject::tr("Meters Per Square Hour");},      []{ return QObject::tr("m/h2");      },     3600.0 * 3600.);
    IMPLEMENT_MEASUREMENT_UNIT(KilometersPerSqHour, []{return QObject::tr("Kilometers Per Square Hour");},  []{ return QObject::tr("km/h2");      },    3600.0 * 3600. / 1000);
    IMPLEMENT_MEASUREMENT_UNIT(FeetsPerSqSec,       []{return QObject::tr("Feets Per Square Second");},     []{ return QObject::tr("ft/s2");      },    1./ METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(FeetsPerSqMinute,    []{return QObject::tr("Feets Per Square Minute");},     []{ return QObject::tr("ft/min2");      },  3600./ METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(FeetsPerSqHour,      []{return QObject::tr("Feets Per Square Hour");},       []{ return QObject::tr("ft/h2");      },    3600. * 3600. / METERS_TO_FEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(USFeetsPerSqSec,     []{return QObject::tr("USFeets Per Square Second");},   []{ return QObject::tr("usft/s2");      },   1. / METERS_TO_USFEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(USFeetsPerSqMinute,  []{return QObject::tr("USFeets Per Square Minute");},   []{ return QObject::tr("usft/m2");      },   3600. / METERS_TO_USFEETS_MULTIPLIER);
    IMPLEMENT_MEASUREMENT_UNIT(USFeetsPerSqHour,    []{return QObject::tr("USFeets Per Square Hour");},     []{ return QObject::tr("usft/h2");      },   3600. * 3600. / METERS_TO_USFEETS_MULTIPLIER);
};

IMPLEMENT_MEASUREMENT(Acceleration, TR(MeasurementTr::tr("Acceleration")))
