#include "externalproperty.h"

#include <SharedGuiModule/internal.hpp>

void ExternalNamedUIntProperty::SetNames(const QStringList& names)
{
    _max = names.size() - 1;
    _names = names;
}

ExternalVector3FProperty::ExternalVector3FProperty(const Name& path, Vector3F& vector)
    : X(path+"/x", vector[0], -FLT_MAX, FLT_MAX)
    , Y(path+"/y", vector[1], -FLT_MAX, FLT_MAX)
    , Z(path+"/z", vector[2], -FLT_MAX, FLT_MAX)
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
