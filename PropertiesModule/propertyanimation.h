#ifndef PROPERTYANIMATION_H
#define PROPERTYANIMATION_H

#include <QVariantAnimation>

class PropertyAnimation : public QVariantAnimation
{
    Q_OBJECT
public:
    PropertyAnimation(class Property* property);

    // QVariantAnimation interface
protected:
    void updateCurrentValue(const QVariant& value) Q_DECL_OVERRIDE;

private:
    Property* m_property;
};

#endif // PROPERTYANIMATION_H
