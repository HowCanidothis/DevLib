#ifndef GTMATERIALFRAMETEXTURE_H
#define GTMATERIALFRAMETEXTURE_H

#ifdef OPENCV

#include "gtmaterialparametertexturebase.h"

class GtMaterialParameterFrameTexture : public GtMaterialParameterTextureBase
{
    typedef GtMaterialParameterTextureBase Super;
    ScopedPointer<GtFrameTextureResource> _frameTexture;
public:
    GtMaterialParameterFrameTexture(const QString& name, const QString& resource);

    // GtMaterialBase interface
protected:
    FDelegate apply() Q_DECL_OVERRIDE;
};

#endif

#endif // GTMATERIALFRAMETEXTURE_H
