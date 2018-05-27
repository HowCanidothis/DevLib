#include "gtmaterialparameterbase.h"

#include <QOpenGLShaderProgram>
#include "gtmaterial.h"
#include "../gtcamera.h"
#include "../gttexture2D.h"

GtMaterialParameterBase::GtMaterialParameterBase(const QString& name, const QString& resource)
    : name(name)
    , resource(resource)
{

}

GtMaterialParameterBase::GtMaterialParameterBase(const QString& name, const GtMaterialParameterBase::FDelegate& delegate)
    : name(name)
    , delegate(delegate)
{

}

GtMaterialParameterBase::~GtMaterialParameterBase()
{

}

GtMaterialParameterBase::FDelegate GtMaterialParameterBase::apply()
{
    return delegate;
}

void GtMaterialParameterBase::updateLocation(QOpenGLShaderProgram* program)
{
    LOGOUT;
    location = program->uniformLocation(name);
    if(location == -1) {
        log.Warning() << "location not found" << name;
    }
}

void GtMaterialParameterBase::bind(QOpenGLShaderProgram* program, OpenGLFunctions* f)
{
    delegate(program, location, f);
}

void GtMaterialParameterBase::installDelegate()
{
    this->delegate = apply();
}
