#ifndef SPEEDDECLARATIONS_H
#define SPEEDDECLARATIONS_H

#include "measurementunitmanager.h"

static const Name MEASUREMENT_SPEED = "Speed";

namespace SpeedUnits
{
    static const MeasurementUnit MetersPerSecond   ("MetersPerSecond",   []{return QObject::tr("Meters Per Second");},   []{ return QObject::tr("m/s");      }, 11811.02364);
    static const MeasurementUnit MetersPerMinute   ("MetersPerMinute",   []{return QObject::tr("Meters Per Minute");},   []{ return QObject::tr("m/min");    }, 196.850394 );
    static const MeasurementUnit MetersPerHour     ("MetersPerHour",     []{return QObject::tr("Meters Per Hour");},     []{ return QObject::tr("m/h");      }, 3.28084    );
	static const MeasurementUnit KilometersPerHour ("KilometersPerHour", []{return QObject::tr("Kilometers Per Hour");}, []{ return QObject::tr("km/h");     }, 3280.839895);
	static const MeasurementUnit FeetPerHour       ("FeetPerHour",       []{return QObject::tr("Feet Per Hour");},       []{ return QObject::tr("ft/h");     }, 1          );
	static const MeasurementUnit USfeetPerHour     ("USfeetPerHour",     []{return QObject::tr("US Feet Per Hour");},    []{ return QObject::tr("usft/h");   }, 1.000002   );
	static const MeasurementUnit FeetPerMinute     ("FeetPerMinute",     []{return QObject::tr("Feet Per Minute");},     []{ return QObject::tr("ft/min");   }, 60         );
	static const MeasurementUnit USfeetPerMinute   ("USfeetPerMinute",   []{return QObject::tr("US Feet Per Minute");},  []{ return QObject::tr("usft/min"); }, 60.00012   );
	static const MeasurementUnit FeetPerSecond     ("FeetPerSecond",     []{return QObject::tr("Feet Per Second");},     []{ return QObject::tr("ft/sec");   }, 3600       );
	static const MeasurementUnit USfeetPerSecond   ("USfeetPerSecond",   []{return QObject::tr("US Feet Per Second");},  []{ return QObject::tr("usft/sec"); }, 3600.0072  );
	static const MeasurementUnit MilesPerHour      ("MilesPerHour",      []{return QObject::tr("Miles Per Hour");},      []{ return QObject::tr("mph");      }, 5280       );
}


#define MEASUREMENT_SPEED_UNIT_TO_BASE(x) \
	MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_SPEED)->FromUnitToBase(x)
#define MEASUREMENT_SPEED_BASE_TO_UNIT(x) \
	MeasurementManager::GetInstance().GetCurrentUnit(MEASUREMENT_SPEED)->FromBaseToUnit(x)
#define MEASUREMENT_SPEED_PRECISION() \
	MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_SPEED)->Precision
#define MEASUREMENT_SPEED_BASE_TO_UNIT_UI(x) \
	QString::number(MEASUREMENT_SPEED_BASE_TO_UNIT(x), 'f', MEASUREMENT_SPEED_PRECISION())

#define MEASUREMENT_SPEED_STRING MeasurementManager::GetInstance().GetMeasurement(MEASUREMENT_SPEED)->CurrentUnitLabel

#endif // SPEEDDECLARATIONS_H
