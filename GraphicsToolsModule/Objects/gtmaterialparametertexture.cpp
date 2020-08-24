#include "gtmaterialparametertexture.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include "../internal.hpp"
#include "ResourcesModule/resourcessystem.h"

GtMaterialParameterTexture::GtMaterialParameterTexture(const QString& name, const QString& resource)
    : Super(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterTexture::apply()
{
    m_texture = ResourcesSystem::GetResource<GtTexture>(this->m_resource);
    if(m_texture != nullptr) {
        gTexID texture = m_texture->Data().Get().GetId();
        gTexTarget target = m_texture->Data().Get().GetTarget();
        return [this, texture, target](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
            f->glActiveTexture(m_unit + GL_TEXTURE0);
            f->glBindTexture(target, texture);
            program->setUniformValue(loc, m_unit);
        };
    }
    return [](QOpenGLShaderProgram* , quint32 , OpenGLFunctions* ){};
}

void GtMaterialParameterTexture::MapProperties(QtObserver* observer)
{
    QString path = "Materials/" + QString::number(m_unit);
    new ExternalStringProperty(Name(path + "/Name"), m_name);
    new ExternalNameProperty(Name(path + "/Resource"), m_resource);

    observer->AddStringObserver(&m_name,[]{ GtMaterialParameterTexture::material()->Update(); });
    observer->AddStringObserver(&m_resource.AsString(), []{ GtMaterialParameterTexture::material()->Update(); });
}
