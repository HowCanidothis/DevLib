#ifndef GTMATERIALMATRIX4X4_H
#define GTMATERIALMATRIX4X4_H

#include "gtmaterialparameterbase.h"

class GtMaterialParameterMatrix : public GtMaterialResourceParameterBase
{
    using Super = GtMaterialResourceParameterBase;
    Matrix4Resource m_matrix;
public:
    GtMaterialParameterMatrix(const QString& m_name, const Name& m_resource);

    // GtObjectBase interface
private:
    virtual FDelegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALMATRIX4X4_H
