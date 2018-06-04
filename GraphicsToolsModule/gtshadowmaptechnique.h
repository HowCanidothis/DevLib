#ifndef GTSHADOWMAPTECHNIQUE_H
#define GTSHADOWMAPTECHNIQUE_H

#include "SharedGuiModule/decl.h"
#include "SharedModule/array.h"

class GtFramebufferObject;
class GtCamera;

class GtShadowMapTechnique
{   
    ScopedPointer<GtCamera> camera;
    ScopedPointer<GtFramebufferObject> framebuffer;
    OpenGLFunctions* f;

public:
    GtShadowMapTechnique(OpenGLFunctions* f, const SizeI& resolution);

    void create();

    void bind(const Point3F& spot_position, const Vector3F& spot_center);
    void release();

    const GtCamera* getCam() const { return camera.data(); }
    const Matrix4& getWorld();
    gTexID getDepthTexture() const;
};

#endif // GTSHADOWMAPTECHNIQUE_H
