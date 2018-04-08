#ifndef GTMATERIALTEXTURE_H
#define GTMATERIALTEXTURE_H

#include "gtmaterialparametertexturebase.h"

class GtMaterialParameterTexture : public GtMaterialParameterTextureBase
{
    typedef GtMaterialParameterTextureBase Super;
    ScopedPointer<GtTextureResource> gt_texture;
public:
    GtMaterialParameterTexture(const QString& name, const QString& resource);

    // GtObjectBase interface
private:
    void mapProperties(Observer* observer) Q_DECL_OVERRIDE;
    virtual FDelegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALTEXTURE_H
