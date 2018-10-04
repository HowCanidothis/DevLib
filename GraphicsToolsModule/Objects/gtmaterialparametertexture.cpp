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
    gt_texture = ResourcesSystem::GetResource<GtTexture>(this->resource);
    if(gt_texture != nullptr) {
        gTexID texture = gt_texture->Data().Get().getID();
        gTexTarget target = gt_texture->Data().Get().getTarget();
        return [this, texture, target](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
            f->glActiveTexture(unit + GL_TEXTURE0);
            f->glBindTexture(target, texture);
            program->setUniformValue(loc, unit);
        };
    }
    return [](QOpenGLShaderProgram* , quint32 , OpenGLFunctions* ){};
}

void GtMaterialParameterTexture::mapProperties(Observer* observer)
{
    QString path = "Materials/" + QString::number(unit);
    new ExternalStringProperty(Name(path + "/Name"), name);
    new ExternalNameProperty(Name(path + "/Resource"), resource);

    observer->AddStringObserver(&name,[]{ GtMaterialParameterTexture::material()->update(); });
    observer->AddStringObserver(&resource.AsString(), []{ GtMaterialParameterTexture::material()->update(); });
}
