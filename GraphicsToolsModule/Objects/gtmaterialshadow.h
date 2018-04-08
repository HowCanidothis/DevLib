#ifndef GTMATERIALSHADOW_H
#define GTMATERIALSHADOW_H

#include "gtmaterialtexturebase.h"

class GtShadowMapTechnique;

class GtMaterialShadow : public GtMaterialTextureBase
{
    typedef GtMaterialTextureBase Super;
    ScopedPointer<GtShadowMapTechniqueResource> technique;
    // GtObjectBase interface
public:
    GtMaterialShadow(const QString& name, const QString& resource);

private:
    virtual F_Delegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALSHADOW_H
