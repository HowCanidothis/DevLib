#include "gtmaterialframetexture.h"

#include <QOpenGLShaderProgram>

#include "../gtframetexture.h"

GtMaterialFrameTexture::GtMaterialFrameTexture(const QString& name, const QString& resource)
    : Super(name, resource)
{

}

GtMaterialFrameTexture::F_Delegate GtMaterialFrameTexture::apply()
{
    frame_texture = ResourcesSystem::getResource<GtFrameTexture>(resource);
    gTexID texture = frame_texture->data()->getOutput()->getID();
    return [this, texture](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
        GtTexture2D::bindTexture(f, unit, texture);
        program->setUniformValue(loc, unit);
    };
}
