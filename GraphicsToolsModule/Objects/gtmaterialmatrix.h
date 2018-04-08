#ifndef GTMATERIALMATRIX4X4_H
#define GTMATERIALMATRIX4X4_H

#include "gtmaterialbase.h"

class GtMaterialMatrix : public GtMaterialBase
{
    typedef GtMaterialBase Super;
    ScopedPointer<Matrix4Resource> matrix;
public:
    GtMaterialMatrix(const QString& name, const QString& resource);

    // GtObjectBase interface
private:
    virtual F_Delegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALMATRIX4X4_H
