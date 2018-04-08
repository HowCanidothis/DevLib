#include "gtmaterialbase.h"

#include <QOpenGLShaderProgram>
#include "gtviewbase.h"
#include "../gtcamera.h"
#include "../gttexture2D.h"

GtMaterialBase::GtMaterialBase(const QString& name, const QString& resource)
    : name(name)
    , resource(resource)
{

}

GtMaterialBase::GtMaterialBase(const QString& name, const GtMaterialBase::F_Delegate& delegate)
    : name(name)
    , delegate(delegate)
{

}

GtMaterialBase::~GtMaterialBase()
{

}

GtMaterialBase::F_Delegate GtMaterialBase::apply()
{
    return delegate;
}

void GtMaterialBase::updateLocation(QOpenGLShaderProgram* program)
{
    LOGOUT;
    location = program->uniformLocation(name);
    if(location == -1) {
        log.warning() << "location not found" << name;
    }
}

void GtMaterialBase::bind(QOpenGLShaderProgram* program, OpenGLFunctions* f)
{
    delegate(program, location, f);
}

void GtMaterialBase::installDelegate()
{
    this->delegate = apply();
}
