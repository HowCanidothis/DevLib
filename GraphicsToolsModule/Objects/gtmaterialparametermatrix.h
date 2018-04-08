#ifndef GTMATERIALMATRIX4X4_H
#define GTMATERIALMATRIX4X4_H

#include "gtmaterialparameterbase.h"

class GtMaterialParameterMatrix : public GtMaterialParameterBase
{
    typedef GtMaterialParameterBase Super;
    ScopedPointer<Matrix4Resource> matrix;
public:
    GtMaterialParameterMatrix(const QString& name, const QString& resource);

    // GtObjectBase interface
private:
    virtual FDelegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALMATRIX4X4_H
