#include "speeddeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace SpeedUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerSecond,   []{return QObject::tr("Meters Per Second");},   []{ return QObject::tr("m/s");      }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerMinute,   []{return QObject::tr("Meters Per Minute");},   []{ return QObject::tr("m/min");    }, 1.0 / 60.0 );
    IMPLEMENT_MEASUREMENT_UNIT(MetersPerHour,     []{return QObject::tr("Meters Per Hour");},     []{ return QObject::tr("m/hr");      }, 1.0 / 60.0 / 60.0 );
    IMPLEMENT_MEASUREMENT_UNIT(KilometersPerHour, []{return QObject::tr("Kilometers Per Hour");}, []{ return QObject::tr("km/hr");     }, 1000.0 / 60.0 / 60.0);
    IMPLEMENT_MEASUREMENT_UNIT(FeetPerHour,       []{return QObject::tr("Feet Per Hour");},       []{ return QObject::tr("ft/hr");     }, 1.0 / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0 );
    IMPLEMENT_MEASUREMENT_UNIT(USfeetPerHour,     []{return QObject::tr("US Feet Per Hour");},    []{ return QObject::tr("usft/hr");   }, USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0 );
    IMPLEMENT_MEASUREMENT_UNIT(FeetPerMinute,     []{return QObject::tr("Feet Per Minute");},     []{ return QObject::tr("ft/min");   }, 1.0 / 60.0 / METERS_TO_FEETS_MULTIPLIER );
    IMPLEMENT_MEASUREMENT_UNIT(USfeetPerMinute,   []{return QObject::tr("US Feet Per Minute");},  []{ return QObject::tr("usft/min"); }, USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER / 60.0   );
    IMPLEMENT_MEASUREMENT_UNIT(FeetPerSecond,     []{return QObject::tr("Feet Per Second");},     []{ return QObject::tr("ft/sec");   }, 1.0/ METERS_TO_FEETS_MULTIPLIER );
    IMPLEMENT_MEASUREMENT_UNIT(USfeetPerSecond,   []{return QObject::tr("US Feet Per Second");},  []{ return QObject::tr("usft/sec"); }, USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER );
    IMPLEMENT_MEASUREMENT_UNIT(MilesPerHour,      []{return QObject::tr("Miles Per Hour");},      []{ return QObject::tr("mphr");      }, 5280 / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0);
}

IMPLEMENT_MEASUREMENT(Speed, TR(MeasurementTr::tr("Speed")))
IMPLEMENT_MEASUREMENT(AnnularVelocity, TR(MeasurementTr::tr("Annular Velocity")))
IMPLEMENT_MEASUREMENT(ROP, TR(MeasurementTr::tr("Rate Of Penetration")))
