#include "gtmaterialtexture.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include "../internal.hpp"
#include "ResourcesModule/resourcessystem.h"

GtMaterialTexture::GtMaterialTexture(const QString& name, const QString& resource)
    : Super(name, resource)
{}

GtMaterialBase::F_Delegate GtMaterialTexture::apply()
{
    gt_texture = ResourcesSystem::getResource<GtTexture>(this->resource);
    if(gt_texture != nullptr) {
        gTexID texture = gt_texture->data()->getID();
        gTexTarget target = gt_texture->data()->getTarget();
        return [this, texture, target](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
            f->glActiveTexture(unit + GL_TEXTURE0);
            f->glBindTexture(target, texture);
            program->setUniformValue(loc, unit);
        };
    }
    return [](QOpenGLShaderProgram* , quint32 , OpenGLFunctions* ){};
}

void GtMaterialTexture::mapProperties(Observer* observer)
{
    QString path = "Materials/" + QString::number(unit);
    new StringPropertyPtr(path + "/Name", &name);
    new StringPropertyPtr(path + "/Resource", &resource);

    observer->addStringObserver(&name,[]{ GtMaterialTexture::view()->update(); });
    observer->addStringObserver(&resource, []{ GtMaterialTexture::view()->update(); });
}
