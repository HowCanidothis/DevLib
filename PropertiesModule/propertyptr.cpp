#include "propertyptr.h"

Vector3FPropertyPtr::Vector3FPropertyPtr(const QString& path, Vector3F* vector)
    : x(path+"/x", &vector->operator [](0), -FLT_MAX, FLT_MAX)
    , y(path+"/y", &vector->operator [](1), -FLT_MAX, FLT_MAX)
    , z(path+"/z", &vector->operator [](2), -FLT_MAX, FLT_MAX)
{

}
