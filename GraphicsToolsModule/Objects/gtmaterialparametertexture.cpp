#include "gtmaterialparametertexture.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include "ResourcesModule/resourcessystem.h"
#include "GraphicsToolsModule/gtrenderer.h"
#include "GraphicsToolsModule/gttexture2D.h"

GtMaterialParameterTexture::GtMaterialParameterTexture(const QString& name, const Name& resource)
    : Super(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterTexture::apply()
{
    m_texture = currentRenderer()->GetResource<GtTexture>(m_resource);
    if(m_texture != nullptr) {
        gTexID textureId;
        gTexTarget target;
        m_texture.GetAccess([&](GtTexture& texture){
            textureId = texture.GetId();
            target = texture.GetTarget();
        });
        return [this, textureId, target](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
            f->glActiveTexture(m_unit + GL_TEXTURE0);
            f->glBindTexture(target, textureId);
            program->setUniformValue(loc, m_unit);
        };
    }
    return [this](QOpenGLShaderProgram* , quint32 , OpenGLFunctions* ){
        qDebug() << "Unable to find resource " << m_resource;
    };
}
