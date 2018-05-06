#include "property.h"
#include "propertiessystem.h"

Property::Property(const QString& path)
    : fonset([]{})
    , fvalidator([](const QVariant&, QVariant&){})
    , read_only(false)
{
    PropertiesSystem::addProperty(Name(path), this);
}

void Property::setValue(QVariant value) // copied as it could be validated
{
    QVariant old_value = getValue();
    if(old_value != value) {
        fvalidator(old_value,value);
        fhandle([this,value] {
            this->setValueInternal(value);
            fonset();
        });
    }
}

Vector3FProperty::Vector3FProperty(const QString& path, const Vector3F& vector)
    : x(path+"/x", vector.x(), -FLT_MAX, FLT_MAX)
    , y(path+"/y", vector.y(), -FLT_MAX, FLT_MAX)
    , z(path+"/z", vector.z(), -FLT_MAX, FLT_MAX)
{

}
