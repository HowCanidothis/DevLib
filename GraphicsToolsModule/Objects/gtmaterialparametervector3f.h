#ifndef GTMATERIALPARAMETERVECTOR3F_H
#define GTMATERIALPARAMETERVECTOR3F_H


#include "gtmaterialparameterbase.h"

class GtMaterialParameterVector3F : public GtMaterialParameterBase
{
    typedef GtMaterialParameterBase Super;
    SharedPointer<Vector3FResource> m_vector;
public:
    GtMaterialParameterVector3F(const QString& m_name, const Name& m_resource);

    // GtObjectBase interface
private:
    virtual FDelegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALPARAMETERVECTOR3F_H
