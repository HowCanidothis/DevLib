#include "property.h"
#include "propertiessystem.h"

Property::Property(const Name& path)
    : m_fOnChange([]{})
    , m_fValidator([](const QVariant&, QVariant&){})
    , m_options(Options_Default)
#ifdef DEBUG_BUILD
    , m_isSubscribed(false)
#endif
{
    PropertiesSystem::addProperty(path, this);
}

bool Property::SetValue(QVariant value) // copied as it could be validated
{
    QVariant oldValue = getValue();
    m_fValidator(oldValue,value);
    if(oldValue != value) {
        m_previousValue = oldValue;
        m_fHandle([this,value] {
            this->setValueInternal(value);
            Invoke();
        });
        return true;
    }
    return false;
}

void Property::Subscribe(const Property::FOnChange& onChange) {
#ifdef DEBUG_BUILD
    m_isSubscribed = true;
#endif
    auto oldHandle = m_fOnChange;
    m_fOnChange = [onChange, oldHandle]{
        oldHandle();
        onChange();
    };
}

void Property::Invoke()
{
#ifdef DEBUG_BUILD
    Q_ASSERT(!_isSubscribed);
#endif
    m_fOnChange();
    m_onChangeDispatcher.Invoke();
}

void NamedUIntProperty::SetNames(const QStringList& names)
{
    m_max = names.size() - 1;
    m_names = names;
}

void UrlListProperty::AddUniqueUrl(const QUrl& url)
{
    auto urlList = Super::m_value; // Copy
    auto find = std::find(urlList.begin(), urlList.end(), url);
    if(find != urlList.end()) {
        urlList.erase(find);
        urlList.push_front(url);
    } else {
        urlList.prepend(url);
        if(m_maxCount != -1) {
            if(urlList.size() > m_maxCount) {
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
