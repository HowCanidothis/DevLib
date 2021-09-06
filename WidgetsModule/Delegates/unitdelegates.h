#ifndef UNITDELEGATES_H
#define UNITDELEGATES_H

#include "delegates.h"

class UnitDelegateBase : public DelegatesDoubleSpinBox 
{
	using Super  = DelegatesDoubleSpinBox;
public:
	UnitDelegateBase(const Name& type, double baseMin, double baseMax, double baseStep, QObject* parent);
	
private:
	DispatcherConnectionsSafe m_connections;
};

class AngleDelegate : public UnitDelegateBase 
{
	using Super  = UnitDelegateBase;
public:
	AngleDelegate(QObject* parent = nullptr); 
	AngleDelegate(double baseMin, double baseMax, QObject* parent = nullptr);
	AngleDelegate(double baseMin, double baseMax, double baseStep, QObject* parent = nullptr); 
};

class DepthDelegate : public UnitDelegateBase 
{
	using Super  = UnitDelegateBase;
public:
	DepthDelegate(QObject* parent = nullptr); 
	DepthDelegate(double baseMin, double baseMax, QObject* parent = nullptr);
	DepthDelegate(double baseMin, double baseMax, double baseStep, QObject* parent = nullptr); 
};

#endif // UNITDELEGATES_H
