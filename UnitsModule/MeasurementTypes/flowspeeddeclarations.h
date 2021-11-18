#ifndef FLOWSPEEDDECLARATIONS_H
#define FLOWSPEEDDECLARATIONS_H

#include "UnitsModule/measurementunitmanager.h"

static const Name MEASUREMENT_FLOW_SPEED = "Flow Speed";

namespace FlowSpeedUnits
{
    static const MeasurementUnit CubicMetersPerSecond ("CubicMetersPerSecond", []{return QObject::tr("cubic meters per second");}, []{ return QObject::tr("m3/s");   }, 1.0);
    static const MeasurementUnit CubicMetersPerMinute ("CubicMetersPerMinute", []{return QObject::tr("cubic meters per minute");}, []{ return QObject::tr("m3/min"); }, 0.0166667  );
    static const MeasurementUnit CubicMetersPerHour   ("CubicMetersPerHour"  , []{return QObject::tr("cubic meters per hour");  }, []{ return QObject::tr("m3/hr");  }, 0.00027778    );
    static const MeasurementUnit CubicMetersPerDay    ("CubicMetersPerDay"   , []{return QObject::tr("cubic meters per day");   }, []{ return QObject::tr("m3/day"); }, 0.00001157    );
    static const MeasurementUnit LitersPerSecond      ("LitersPerSecond"     , []{return QObject::tr("liters per second");      }, []{ return QObject::tr("l/s");    }, 0.001      );
    static const MeasurementUnit LitersPerMinute      ("LitersPerMinute"     , []{return QObject::tr("liters per minute");      }, []{ return QObject::tr("l/min");  }, 0.00001667       );
    static const MeasurementUnit GallonsPerMinute     ("GallonsPerMinute"    , []{return QObject::tr("gallons per minute");     }, []{ return QObject::tr("gpm");}, 0.0000630902            );
    static const MeasurementUnit BarrelsPerMinute     ("BarrelsPerMinute"    , []{return QObject::tr("barrels per minute");     }, []{ return QObject::tr("bbl/min");}, 0.00264979   );
}

#define MEASUREMENT_FLOW_SPEED_STRING              MEASUREMENT_STRING(MEASUREMENT_FLOW_SPEED)
#define MEASUREMENT_FLOW_SPEED_PRECISION           MEASUREMENT_PRECISION(MEASUREMENT_FLOW_SPEED)
#define MEASUREMENT_FLOW_SPEED_UNIT_TO_BASE(x)     MEASUREMENT_UNIT_TO_BASE(MEASUREMENT_FLOW_SPEED, x)
#define MEASUREMENT_FLOW_SPEED_BASE_TO_UNIT(x)     MEASUREMENT_BASE_TO_UNIT(MEASUREMENT_FLOW_SPEED, x)
#define MEASUREMENT_FLOW_SPEED_BASE_TO_UNIT_UI(x)  MEASUREMENT_BASE_TO_UNIT_UI(MEASUREMENT_FLOW_SPEED, x)
#define MEASUREMENT_FLOW_SPEED_DISPATCHER          MEASUREMENT_DISPATCHER(MEASUREMENT_FLOW_SPEED)

#define ATTACH_FLOW_SPEED_MEASUREMENT(delegate, min, max) ATTACH_MEASUREMENT(MEASUREMENT_FLOW_SPEED, delegate, min, max)MEASUREMENT_FLOW_SPEED


#endif // FLOWSPEEDDECLARATIONS_H
