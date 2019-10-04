#include "gtmaterialparameterbase.h"

#include <QOpenGLShaderProgram>
#include "gtmaterial.h"
#include "../gtcamera.h"
#include "../gttexture2D.h"

GtMaterialParameterBase::GtMaterialParameterBase(const QString& name, const QString& resource)
    : m_name(name)
    , m_resource(resource)
{

}

GtMaterialParameterBase::GtMaterialParameterBase(const QString& name, const GtMaterialParameterBase::FDelegate& delegate)
    : m_name(name)
    , m_delegate(delegate)
{

}

GtMaterialParameterBase::~GtMaterialParameterBase()
{

}

GtMaterialParameterBase::FDelegate GtMaterialParameterBase::apply()
{
    return m_delegate;
}

void GtMaterialParameterBase::updateLocation(QOpenGLShaderProgram* program)
{
    m_location = program->uniformLocation(m_name);
    if(m_location == -1) {
        qCWarning(LC_SYSTEM) << "location not found" << m_name;
    }
}

void GtMaterialParameterBase::bind(QOpenGLShaderProgram* program, OpenGLFunctions* f)
{
    m_delegate(program, m_location, f);
}

void GtMaterialParameterBase::installDelegate()
{
    this->m_delegate = apply();
}
