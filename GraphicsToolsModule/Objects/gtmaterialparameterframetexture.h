#ifndef GTMATERIALFRAMETEXTURE_H
#define GTMATERIALFRAMETEXTURE_H

#include "gtmaterialparametertexturebase.h"

class GtMaterialParameterFrameTexture : public GtMaterialParameterTextureBase
{
    typedef GtMaterialParameterTextureBase Super;
    ScopedPointer<GtFrameTextureResource> frame_texture;
public:
    GtMaterialParameterFrameTexture(const QString& name, const QString& resource);

    // GtMaterialBase interface
protected:
    FDelegate apply() Q_DECL_OVERRIDE;
};

#endif // GTMATERIALFRAMETEXTURE_H
