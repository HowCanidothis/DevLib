#include "currencydeclarations.h"

#include "UnitsModule/measurementunitmanager.h"

namespace CurrencyUnits
{
    IMPLEMENT_MEASUREMENT_UNIT(Dollar, TR(MeasurementTr::tr("United States dollar")), TR(MeasurementTr::tr("$")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(PoundSterling, TR(MeasurementTr::tr("Pound sterling")), TR(MeasurementTr::tr("£")), 1.0)
    IMPLEMENT_MEASUREMENT_UNIT(Euro, TR(MeasurementTr::tr("Euro ")), TR(MeasurementTr::tr("€")), 1.)
    IMPLEMENT_MEASUREMENT_UNIT(Yuan, TR(MeasurementTr::tr("Renminbi")), TR(MeasurementTr::tr("¥")), 1.)
    IMPLEMENT_MEASUREMENT_UNIT(Ruble, TR(MeasurementTr::tr("Ruble")), TR(MeasurementTr::tr("₽")), 1.)
};
IMPLEMENT_MEASUREMENT(Currency, TR(MeasurementTr::tr("Currency")))
