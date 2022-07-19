#include "flowspeeddeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace FlowSpeedUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerSecond, []{return QObject::tr("cubic meters per second");}, []{ return QObject::tr("m3/s");   }, 1.0);
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerMinute, []{return QObject::tr("cubic meters per minute");}, []{ return QObject::tr("m3/min"); }, 0.0166667);
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerHour, []{return QObject::tr("cubic meters per hour");  }, []{ return QObject::tr("m3/hr");  }, 0.00027778);
    IMPLEMENT_MEASUREMENT_UNIT(CubicMetersPerDay, []{return QObject::tr("cubic meters per day");   }, []{ return QObject::tr("m3/day"); }, 0.00001157);
    IMPLEMENT_MEASUREMENT_UNIT(LitersPerSecond, []{return QObject::tr("liters per second");      }, []{ return QObject::tr("l/s");    }, 0.001);
    IMPLEMENT_MEASUREMENT_UNIT(LitersPerMinute, []{return QObject::tr("liters per minute");      }, []{ return QObject::tr("l/min");  }, 0.00001667);
    IMPLEMENT_MEASUREMENT_UNIT(GallonsPerMinute, []{return QObject::tr("gallons per minute");     }, []{ return QObject::tr("gpm");}, 0.0000630902);
    IMPLEMENT_MEASUREMENT_UNIT(BarrelsPerMinute, []{return QObject::tr("barrels per minute");     }, []{ return QObject::tr("bbl/min");}, 0.00264979);
    IMPLEMENT_MEASUREMENT_UNIT(CubicFeetPerSecond, []{return QObject::tr("Cubic feet per second");     }, []{ return QObject::tr("ft³/s");}, 0.0283168466);
}

IMPLEMENT_MEASUREMENT(FlowSpeed)
