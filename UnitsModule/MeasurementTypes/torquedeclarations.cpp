#include "torquedeclarations.h"
#include "UnitsModule/measurementunitmanager.h"

namespace TorqueUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(NewtonMeters, []{return QObject::tr("newton meters");       }, []{ return QObject::tr("Nm");    }, 1);
    IMPLEMENT_MEASUREMENT_UNIT(DecaNewtonMeters, []{return QObject::tr("decanewton meters");       }, []{ return QObject::tr("daNm");    }, 10.0);
    IMPLEMENT_MEASUREMENT_UNIT(KilonewtonMeters, []{return QObject::tr("kilonewton meters");   }, []{ return QObject::tr("kNm");   }, 1000.0 );
    IMPLEMENT_MEASUREMENT_UNIT(PoundForceFeet, []{return QObject::tr("pound-force feet");    }, []{ return QObject::tr("ftlbf"); }, 1.355817952003);
    IMPLEMENT_MEASUREMENT_UNIT(KilopoundForceFeet, []{return QObject::tr("kilopound-force feet");}, []{ return QObject::tr("kftlbs");}, 1355.817952);
    IMPLEMENT_MEASUREMENT_UNIT(PoundFoot, []{return QObject::tr("pound-foot");}, []{ return QObject::tr("lbf*ft");}, 0.73756214927727);
}

IMPLEMENT_MEASUREMENT(Torque)
