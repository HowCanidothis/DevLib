#include "gtshadowmaptechnique.h"

#include "gtcamera.h"
#include "gtframebufferobject.h"

GtShadowMapTechnique::GtShadowMapTechnique(OpenGLFunctions* f, const SizeI& resolution)
    : m_camera(new GtCamera)
    , m_framebuffer(new GtFramebufferObject(f, resolution))
    , f(f)
{
    m_camera->SetIsometric(true);
    m_camera->Resize(resolution.width(), resolution.height());
    m_camera->SetProjectionProperties(45.f, 1.f, 30000.f);
}

void GtShadowMapTechnique::Create()
{
    GtFramebufferFormat format;
    format.SetDepthAttachment(GtFramebufferFormat::Texture);
    m_framebuffer->Create(format);
}

void GtShadowMapTechnique::Bind(const Point3F& spot_position, const Vector3F& spot_center)
{
    m_camera->SetPosition(spot_position, spot_center);
    m_framebuffer->Bind();
}

void GtShadowMapTechnique::Release()
{
    m_framebuffer->Release();
}

const Matrix4& GtShadowMapTechnique::GetWorldMatrix()
{
    return m_camera->GetWorld();
}

gTexID GtShadowMapTechnique::GetDepthTexture() const
{
    return m_framebuffer->GetDepthTexture()->GetId();
}
