#ifndef GTMATERIALTEXTURE_H
#define GTMATERIALTEXTURE_H

#include "gtmaterialtexturebase.h"

class GtMaterialTexture : public GtMaterialTextureBase
{
    typedef GtMaterialTextureBase Super;
    ScopedPointer<GtTextureResource> gt_texture;
public:
    GtMaterialTexture(const QString& name, const QString& resource);

    // GtObjectBase interface
private:
    void mapProperties(Observer* observer) Q_DECL_OVERRIDE;
    virtual F_Delegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALTEXTURE_H
