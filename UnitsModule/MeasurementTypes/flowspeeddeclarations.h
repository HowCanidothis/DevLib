#ifndef FLOWSPEEDDECLARATIONS_H
#define FLOWSPEEDDECLARATIONS_H

#include "measurementunitmanager.h"

static const Name MEASUREMENT_FLOW_SPEED = "Flow Speed";

namespace FlowSpeedUnits
{
    static const MeasurementUnit CubicMetersPerSecond ("CubicMetersPerSecond", []{return QObject::tr("cubic meters per second");}, []{ return QObject::tr("m3/s");   }, 15850.3306115);
    static const MeasurementUnit CubicMetersPerMinute ("CubicMetersPerMinute", []{return QObject::tr("cubic meters per minute");}, []{ return QObject::tr("m3/min"); }, 264.1721769  );
    static const MeasurementUnit CubicMetersPerHour   ("CubicMetersPerHour"  , []{return QObject::tr("cubic meters per hour");  }, []{ return QObject::tr("m3/hr");  }, 4.4028696    );
    static const MeasurementUnit CubicMetersPerDay    ("CubicMetersPerDay"   , []{return QObject::tr("cubic meters per day");   }, []{ return QObject::tr("m3/day"); }, 0.1834529    );
    static const MeasurementUnit LitersPerSecond      ("LitersPerSecond"     , []{return QObject::tr("liters per second");      }, []{ return QObject::tr("l/s");    }, 15.8503      );
    static const MeasurementUnit LitersPerMinute      ("LitersPerMinute"     , []{return QObject::tr("liters per minute");      }, []{ return QObject::tr("l/min");  }, 0.2642       );
    static const MeasurementUnit GallonsPerMinute     ("GallonsPerMinute"    , []{return QObject::tr("gallons per minute");     }, []{ return QObject::tr("gal/min");}, 1            );
	static const MeasurementUnit BarrelsPerMinute     ("BarrelsPerMinute"    , []{return QObject::tr("barrels per minute");     }, []{ return QObject::tr("bbl/min");}, 42.0000211   );
}

#define MEASUREMENT_FLOW_SPEED_STRING              MEASUREMENT_STRING(MEASUREMENT_FLOW_SPEED)
#define MEASUREMENT_FLOW_SPEED_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_FLOW_SPEED)
#define MEASUREMENT_FLOW_SPEED_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_FLOW_SPEED, x)
#define MEASUREMENT_FLOW_SPEED_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_FLOW_SPEED, x)
#define MEASUREMENT_FLOW_SPEED_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_FLOW_SPEED, x)


#endif // FLOWSPEEDDECLARATIONS_H
