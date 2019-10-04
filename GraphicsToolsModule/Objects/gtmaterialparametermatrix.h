#ifndef GTMATERIALMATRIX4X4_H
#define GTMATERIALMATRIX4X4_H

#include "gtmaterialparameterbase.h"

class GtMaterialParameterMatrix : public GtMaterialParameterBase
{
    typedef GtMaterialParameterBase Super;
    ScopedPointer<Matrix4Resource> m_matrix;
public:
    GtMaterialParameterMatrix(const QString& m_name, const QString& m_resource);

    // GtObjectBase interface
private:
    virtual FDelegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALMATRIX4X4_H
