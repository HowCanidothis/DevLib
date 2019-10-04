#include "externalproperty.h"

#include <limits>

#include <SharedGuiModule/internal.hpp>

void ExternalNamedUIntProperty::SetNames(const QStringList& names)
{
    _max = names.size() - 1;
    _names = names;
}

#ifdef QT_GUI_LIB

ExternalVector3FProperty::ExternalVector3FProperty(const QString& path, Vector3F& vector)
    : X(Name(path + "/x"), vector.X(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
    , Y(Name(path + "/y"), vector.Y(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
    , Z(Name(path + "/z"), vector.Z(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())
{

}

void ExternalVector3FProperty::Subscribe(const Property::FOnChange& handle)
{
    X.Subscribe(handle);
    Y.Subscribe(handle);
    Z.Subscribe(handle);
}

void ExternalVector3FProperty::SetReadOnly(bool readOnly)
{
    if(readOnly) {
        X.ChangeOptions().AddFlag(Property::Option_IsReadOnly);
        Y.ChangeOptions().AddFlag(Property::Option_IsReadOnly);
        Z.ChangeOptions().AddFlag(Property::Option_IsReadOnly);
    } else {
        X.ChangeOptions().RemoveFlag(Property::Option_IsReadOnly);
        Y.ChangeOptions().RemoveFlag(Property::Option_IsReadOnly);
        Z.ChangeOptions().RemoveFlag(Property::Option_IsReadOnly);
    }
}

#endif
