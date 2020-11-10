#ifndef GTMATERIALPARAMETERVECTOR3F_H
#define GTMATERIALPARAMETERVECTOR3F_H


#include "gtmaterialparameterbase.h"

class GtMaterialParameterVector2F : public GtMaterialResourceParameterBase
{
    using Super = GtMaterialResourceParameterBase;
    SharedPointer<Vector2FResource> m_vector;
public:
    GtMaterialParameterVector2F(const QString& m_name, const Name& m_resource);

    // GtObjectBase interface
private:
    virtual FDelegate apply() override;
};

class GtMaterialParameterVector3F : public GtMaterialResourceParameterBase
{
    using Super = GtMaterialResourceParameterBase;
    SharedPointer<Vector3FResource> m_vector;
public:
    GtMaterialParameterVector3F(const QString& m_name, const Name& m_resource);

    // GtObjectBase interface
private:
    virtual FDelegate apply() override;
};

#endif // GTMATERIALPARAMETERVECTOR3F_H
