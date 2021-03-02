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
    m_texture = currentRenderer()->GetResource<GtTexture>(this->m_resource);
    if(m_texture != nullptr) {
        gTexID texture = m_texture->Data().Get().GetId();
        gTexTarget target = m_texture->Data().Get().GetTarget();
        return [this, texture, target](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
            f->glActiveTexture(m_unit + GL_TEXTURE0);
            f->glBindTexture(target, texture);
            program->setUniformValue(loc, m_unit);
        };
    }
    return [this](QOpenGLShaderProgram* , quint32 , OpenGLFunctions* ){
        qDebug() << "Unable to find resource " << m_resource;
    };
}
