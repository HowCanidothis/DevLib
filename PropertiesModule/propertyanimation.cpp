#include "propertyanimation.h"

#include "property.h"

PropertyAnimation::PropertyAnimation(Property* property)
    : m_property(property)
{
}

void PropertyAnimation::updateCurrentValue(const QVariant& value)
{
    m_property->SetValue(value);
}
