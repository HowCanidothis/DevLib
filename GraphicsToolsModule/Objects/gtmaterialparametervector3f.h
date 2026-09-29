#ifndef GTMATERIALPARAMETERVECTOR3F_H
#define GTMATERIALPARAMETERVECTOR3F_H


#include "gtmaterialparameterbase.h"

class GtMaterialParameterVector2F : public GtMaterialResourceParameterBase
{
    using Super = GtMaterialResourceParameterBase;
    Vector2FResource m_vector;
public:
    GtMaterialParameterVector2F(const QString& m_name, const Name& m_resource);

    // GtObjectBase interface
private:
    virtual FDelegate apply() override;
};

class GtMaterialParameterVector3F : public GtMaterialResourceParameterBase
{
    using Super = GtMaterialResourceParameterBase;
    Vector3FResource m_vector;
public:
    GtMaterialParameterVector3F(const QString& m_name, const Name& m_resource);

    // GtObjectBase interface
private:
    virtual FDelegate apply() override;
};

class GtMaterialParameterVector2FArray : public GtMaterialResourceParameterBase
{
    using Super = GtMaterialResourceParameterBase;
    GtVector2FArrayResource m_gpuData;
    int m_totalCount = 0;
public:
    GtMaterialParameterVector2FArray(const QString& name, const Name& resource);

private:
    FDelegate apply() override;
};

#endif // GTMATERIALPARAMETERVECTOR3F_H
