#ifndef GTMATERIALTEXTURE_H
#define GTMATERIALTEXTURE_H

#include "gtmaterialparametertexturebase.h"

class GtMaterialParameterTexture : public GtMaterialParameterTextureBase
{
    typedef GtMaterialParameterTextureBase Super;
    GtTextureResource m_texture;
public:
    GtMaterialParameterTexture(const QString& m_name, const Name& m_resource);

    void MapProperties(QtObserver* ) override {}
    // GtObjectBase interface
private:
    virtual FDelegate apply() override;
};

#endif // GTMATERIALTEXTURE_H
