#ifndef SPEEDDECLARATIONS_H
#define SPEEDDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_SPEED = "Speed";
static const Name MEASUREMENT_ANNULAR_VELOCITY = "Annular Velocity";

#define AnnularVelocityUnits SpeedUnits
namespace SpeedUnits
{
    static const MeasurementUnit MetersPerSecond   ("MetersPerSecond",   []{return QObject::tr("Meters Per Second");},   []{ return QObject::tr("m/s");      }, 1.0);
    static const MeasurementUnit MetersPerMinute   ("MetersPerMinute",   []{return QObject::tr("Meters Per Minute");},   []{ return QObject::tr("m/min");    }, 1.0 / 60.0 );
    static const MeasurementUnit MetersPerHour     ("MetersPerHour",     []{return QObject::tr("Meters Per Hour");},     []{ return QObject::tr("m/hr");      }, 1.0 / 60.0 / 60.0 );
    static const MeasurementUnit KilometersPerHour ("KilometersPerHour", []{return QObject::tr("Kilometers Per Hour");}, []{ return QObject::tr("km/hr");     }, 1000.0 / 60.0 / 60.0);
    static const MeasurementUnit FeetPerHour       ("FeetPerHour",       []{return QObject::tr("Feet Per Hour");},       []{ return QObject::tr("ft/hr");     }, 1.0 / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0 );
    static const MeasurementUnit USfeetPerHour     ("USfeetPerHour",     []{return QObject::tr("US Feet Per Hour");},    []{ return QObject::tr("usft/hr");   }, USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0 );
    static const MeasurementUnit FeetPerMinute     ("FeetPerMinute",     []{return QObject::tr("Feet Per Minute");},     []{ return QObject::tr("ft/min");   }, 1.0 / 60.0 / METERS_TO_FEETS_MULTIPLIER );
    static const MeasurementUnit USfeetPerMinute   ("USfeetPerMinute",   []{return QObject::tr("US Feet Per Minute");},  []{ return QObject::tr("usft/min"); }, USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER / 60.0   );
    static const MeasurementUnit FeetPerSecond     ("FeetPerSecond",     []{return QObject::tr("Feet Per Second");},     []{ return QObject::tr("ft/sec");   }, 1.0/ METERS_TO_FEETS_MULTIPLIER );
    static const MeasurementUnit USfeetPerSecond   ("USfeetPerSecond",   []{return QObject::tr("US Feet Per Second");},  []{ return QObject::tr("usft/sec"); }, USFEETS_TO_FEETS_MULTIPLIER / METERS_TO_FEETS_MULTIPLIER );
    static const MeasurementUnit MilesPerHour      ("MilesPerHour",      []{return QObject::tr("Miles Per Hour");},      []{ return QObject::tr("mphr");      }, 5280 / METERS_TO_FEETS_MULTIPLIER / 60.0 / 60.0);
}

#define MEASUREMENT_SPEED_STRING              MEASUREMENT_STRING(MEASUREMENT_SPEED)
#define MEASUREMENT_SPEED_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_SPEED)
#define MEASUREMENT_SPEED_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_SPEED, x)
#define MEASUREMENT_SPEED_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_SPEED, x)
#define MEASUREMENT_SPEED_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_SPEED, x)
#define MEASUREMENT_SPEED_DISPATCHER          MEASUREMENT_DISPATCHER(MEASUREMENT_SPEED)
#define ATTACH_SPEED_MEASUREMENT(delegate, min, max) ATTACH_MEASUREMENT(MEASUREMENT_SPEED, delegate, min, max)

#endif // SPEEDDECLARATIONS_H
