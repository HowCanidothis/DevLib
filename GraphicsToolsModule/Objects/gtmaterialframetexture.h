#ifndef GTMATERIALFRAMETEXTURE_H
#define GTMATERIALFRAMETEXTURE_H

#include "gtmaterialtexturebase.h"

class GtMaterialFrameTexture : public GtMaterialTextureBase
{
    typedef GtMaterialTextureBase Super;
    ScopedPointer<GtFrameTextureResource> frame_texture;
public:
    GtMaterialFrameTexture(const QString& name, const QString& resource);

    // GtMaterialBase interface
protected:
    F_Delegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALFRAMETEXTURE_H
