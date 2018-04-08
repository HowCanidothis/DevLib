#ifndef GTMATERIALSHADOW_H
#define GTMATERIALSHADOW_H

#include "gtmaterialparametertexturebase.h"

class GtShadowMapTechnique;

class GtMaterialParameterShadow : public GtMaterialParameterTextureBase
{
    typedef GtMaterialParameterTextureBase Super;
    ScopedPointer<GtShadowMapTechniqueResource> technique;
    // GtObjectBase interface
public:
    GtMaterialParameterShadow(const QString& name, const QString& resource);

private:
    virtual FDelegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALSHADOW_H
