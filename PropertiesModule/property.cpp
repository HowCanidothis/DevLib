#include "property.h"
#include "propertiessystem.h"
#include "externalproperty.h"

Property::Property(const Name& path, Options options)
    : m_fOnChange([]{})
    , m_fValidator([](const QVariant&, QVariant&){})
    , m_options(options)
    , m_propertyName(path)
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
    // Q_ASSERT(!m_isSubscribed);
#endif
    m_fOnChange();
    OnChanged.Invoke();
}

SharedPointer<ExternalPropertyProperty> Property::Clone(const Name& newName) const
{
    return ::make_shared<ExternalPropertyProperty>(newName, const_cast<Property*>(this));
}

QVariant Property::GetValueFromRole(int role) const
{
    switch (role) {
    case RoleQmlValue:
    case Qt::DisplayRole: {
        return getDisplayValue();
    }
    case Qt::EditRole: {
        return getValue();
    }
    case Qt::CheckStateRole: {
        if(getValue().type() == QVariant::Bool) {
            return getValue().toBool() ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    case RoleHeaderItem:
        return false;
    case RoleMinValue: {
        return GetMin();
    }
    case RoleMaxValue: {
        return GetMax();
    }
    case RoleDelegateValue: {
        return GetDelegateValue();
    }
    case RoleDelegateData: {
        auto delegateData = GetDelegateData();
        return (delegateData != nullptr) ? *delegateData : QVariant();
    }
    default:
        break;
    }
    return QVariant();
}

DispatcherConnection Property::ConnectAction(const char* locationInfo, const FAction& action) const
{
    return OnChanged.Connect(locationInfo, action);
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
        if(find == urlList.begin()) {
            return;
        }
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

Vector3FProperty::Vector3FProperty(const QString& path, const Vector3F& vector, Property::Options options)
    : X(Name(path+"/x"), vector.x(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), options)
    , Y(Name(path+"/y"), vector.y(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), options)
    , Z(Name(path+"/z"), vector.z(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), options)
{
    auto emitDispatcher = [this]() {
        OnChanged();
    };
    X.Subscribe(emitDispatcher);
    Y.Subscribe(emitDispatcher);
    Z.Subscribe(emitDispatcher);
}

#endif
