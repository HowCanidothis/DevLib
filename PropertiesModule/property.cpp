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

void NamedUIntProperty::SetNames(const QStringList& names)
{
    _max = names.size() - 1;
    _names = names;
}

void UrlListProperty::AddUniqueUrl(const QUrl& url)
{
    auto urlList = Super::_value; // Copy
    auto find = std::find(urlList.begin(), urlList.end(), url);
    if(find != urlList.end()) {
        urlList.erase(find);
        urlList.push_front(url);
    } else {
        urlList.prepend(url);
        if(_maxCount != -1) {
            if(urlList.size() > _maxCount) {
                urlList.pop_back();
            }
        }
    }
    SetValue(QUrl::toStringList(urlList));
}

#ifdef QT_GUI_LIB

Vector3FProperty::Vector3FProperty(const QString& path, const Vector3F& vector)
    : X(Name(path+"/x"), vector.x(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
    , Y(Name(path+"/y"), vector.y(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
    , Z(Name(path+"/z"), vector.z(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
{

}

#endif
