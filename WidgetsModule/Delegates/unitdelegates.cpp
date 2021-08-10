#include "unitdelegates.h"
#include <UnitsModule/internal.hpp>

UnitDelegateBase::UnitDelegateBase(const Name& type, double baseMin, double baseMax, double baseStep, QObject* parent)
	: Super(MEASUREMENT_BASE_TO_UNIT(type, baseMin),
			MEASUREMENT_BASE_TO_UNIT(type, baseMax),
			MEASUREMENT_BASE_TO_UNIT(type, baseStep),
			MEASUREMENT_PRECISION(type), parent)
{
	MeasurementManager::GetInstance().GetMeasurement(type)->OnChanged.Connect(this, [type, baseMin, baseMax, baseStep, this]{
		SetRange(MEASUREMENT_BASE_TO_UNIT(type, baseMin), MEASUREMENT_BASE_TO_UNIT(type, baseMax));
		m_step = MEASUREMENT_BASE_TO_UNIT(type, baseStep);
		m_precision = MEASUREMENT_PRECISION(type);
	}).MakeSafe(m_connections);
}

AngleDelegate::AngleDelegate(double baseMin, double baseMax, double baseStep, QObject* parent)
	: UnitDelegateBase(MEASUREMENT_ANGLES, baseMin, baseMax, baseStep, parent)
{}

AngleDelegate::AngleDelegate(QObject* parent)
	: AngleDelegate(0, 2*M_PI, parent)
{}

AngleDelegate::AngleDelegate(double baseMin, double baseMax, QObject* parent)
	: AngleDelegate(baseMin, baseMax, M_PI/180, parent)
{}

DepthDelegate::DepthDelegate(double baseMin, double baseMax, double baseStep, QObject* parent)
	: UnitDelegateBase(MEASUREMENT_DISTANCES, baseMin, baseMax, baseStep, parent)
{}

DepthDelegate::DepthDelegate(double baseMin, double baseMax, QObject* parent)
	: DepthDelegate(baseMin, baseMax, 1, parent)
{}

DepthDelegate::DepthDelegate(QObject* parent)
	: DepthDelegate(0.0, 50000.0, parent)
{}
