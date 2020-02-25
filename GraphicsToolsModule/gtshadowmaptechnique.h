 #ifndef GTSHADOWMAPTECHNIQUE_H
#define GTSHADOWMAPTECHNIQUE_H

#include "SharedGuiModule/decl.h"
#include "SharedModule/array.h"

class GtFramebufferObject;
class GtCamera;

class GtShadowMapTechnique
{   
    ScopedPointer<GtCamera> m_camera;
    ScopedPointer<GtFramebufferObject> m_framebuffer;
    OpenGLFunctions* f;

public:
    GtShadowMapTechnique(OpenGLFunctions* f, const SizeI& resolution);

    void Create();

    void Bind(const Point3F& spot_position, const Vector3F& spot_center);
    void Release();

    const GtCamera* GetCamera() const { return m_camera.data(); }
    const Matrix4& GetWorldMatrix();
    gTexID GetDepthTexture() const;
};

#endif // GTSHADOWMAPTECHNIQUE_H
