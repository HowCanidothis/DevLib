#ifndef GTMATERIALSHADOW_H
#define GTMATERIALSHADOW_H

#include "gtmaterialparametertexturebase.h"

class GtShadowMapTechnique;

class GtMaterialParameterShadow : public GtMaterialParameterTextureBase
{
    typedef GtMaterialParameterTextureBase Super;
    GtShadowMapTechniqueResource m_technique;
    // GtObjectBase interface
public:
    GtMaterialParameterShadow(const QString& m_name, const Name& m_resource);

private:
    virtual FDelegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALSHADOW_H
