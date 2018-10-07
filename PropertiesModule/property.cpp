#include "property.h"
#include "propertiessystem.h"

Property::Property(const Name& path)
    : _fOnChange([]{})
    , _fValidator([](const QVariant&, QVariant&){})
    , _options(Options_Default)
#ifdef DEBUG_BUILD
    , _isSubscribed(false)
#endif
{
    PropertiesSystem::addProperty(path, this);
}

bool Property::SetValue(QVariant value) // copied as it could be validated
{
    QVariant oldValue = getValue();
    _fValidator(oldValue,value);
    if(oldValue != value) {
        _previousValue = oldValue;
        _fHandle([this,value] {
            this->setValueInternal(value);
            _fOnChange();
        });
        return true;
    }
    return false;
}

Property::FOnChange& Property::OnChange()
{
#ifdef DEBUG_BUILD
    Q_ASSERT(!_isSubscribed);
#endif
    return _fOnChange;
}

void Property::Subscribe(const Property::FOnChange& onChange) {
#ifdef DEBUG_BUILD
    _isSubscribed = true;
#endif
    auto oldHandle = _fOnChange;
    _fOnChange = [onChange, oldHandle]{
        oldHandle();
        onChange();
    };
}

Vector3FProperty::Vector3FProperty(const QString& path, const Vector3F& vector)
    : X(Name(path+"/x"), vector.x(), -FLT_MAX, FLT_MAX)
    , Y(Name(path+"/y"), vector.y(), -FLT_MAX, FLT_MAX)
    , Z(Name(path+"/z"), vector.z(), -FLT_MAX, FLT_MAX)
{

}

void NamedUIntProperty::SetNames(const QStringList& names)
{
    _max = names.size() - 1;
    _names = names;
}
