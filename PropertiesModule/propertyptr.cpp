#include "propertyptr.h"

Vector3FPropertyPtr::Vector3FPropertyPtr(const QString& path, Vector3F* vector)
    : X(path+"/x", &vector->operator [](0), -FLT_MAX, FLT_MAX)
    , Y(path+"/y", &vector->operator [](1), -FLT_MAX, FLT_MAX)
    , Z(path+"/z", &vector->operator [](2), -FLT_MAX, FLT_MAX)
{

}
