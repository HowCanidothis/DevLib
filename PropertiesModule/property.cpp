#include "property.h"
#include "propertiessystem.h"

Property::Property(const QString& path)
    : _fOnset([]{})
    , _fValidator([](const QVariant&, QVariant&){})
    , _bReadOnly(false)
{
    PropertiesSystem::addProperty(Name(path), this);
}

void Property::SetValue(QVariant value) // copied as it could be validated
{
    QVariant oldValue = getValue();
    if(oldValue != value) {
        _fValidator(oldValue,value);
        _fHandle([this,value] {
            this->setValueInternal(value);
            _fOnset();
        });
    }
}

Vector3FProperty::Vector3FProperty(const QString& path, const Vector3F& vector)
    : X(path+"/x", vector.x(), -FLT_MAX, FLT_MAX)
    , Y(path+"/y", vector.y(), -FLT_MAX, FLT_MAX)
    , Z(path+"/z", vector.z(), -FLT_MAX, FLT_MAX)
{

}

void NamedUIntProperty::SetNames(const QStringList& names)
{
    _max = names.size() - 1;
    _names = names;
}
