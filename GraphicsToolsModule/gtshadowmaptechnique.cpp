#include "gtshadowmaptechnique.h"

#include "gtcamera.h"
#include "gtframebufferobject.h"

GtShadowMapTechnique::GtShadowMapTechnique(OpenGLFunctions* f, const SizeI& resolution)
    : camera(new GtCamera)
    , framebuffer(new GtFramebufferObject(f, resolution))
    , f(f)
{
    camera->setIsometric(true);
    camera->resize(resolution.width(), resolution.height());
    camera->setProjectionProperties(45.f, 1.f, 30000.f);
}

void GtShadowMapTechnique::create()
{
    GtFramebufferFormat format;
    format.setDepthAttachment(GtFramebufferFormat::Texture);
    framebuffer->create(format);
}

void GtShadowMapTechnique::bind(const Point3F& spot_position, const Vector3F& spot_center)
{
    camera->setPosition(spot_position, spot_center);
    framebuffer->bind();
}

void GtShadowMapTechnique::release()
{
    framebuffer->release();
}

const Matrix4& GtShadowMapTechnique::getWorld()
{
    return camera->getWorld();
}

gTexID GtShadowMapTechnique::getDepthTexture() const
{
    return framebuffer->getDepthTexture()->getID();
}
