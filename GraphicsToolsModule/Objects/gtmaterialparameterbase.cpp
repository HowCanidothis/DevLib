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
    : m_delegate(delegate)
    , m_name(name)
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
    auto it = m_locations.insert(program, program->uniformLocation(m_name));
    if(*it == -1) {
        qCWarning(LC_SYSTEM) << "location not found" << m_name << "for shaders:";
        for(const auto* shader : program->shaders()) {
            qCWarning(LC_SYSTEM) << shader->sourceCode();
        }
    }
}

void GtMaterialParameterBase::bind(QOpenGLShaderProgram* program, OpenGLFunctions* f)
{
    m_delegate(program, m_locations.value(program, 0), f);
}

void GtMaterialParameterBase::installDelegate()
{
    this->m_delegate = apply();
}
