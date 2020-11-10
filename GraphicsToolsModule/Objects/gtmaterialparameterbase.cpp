#include "gtmaterialparameterbase.h"

#include <QOpenGLShaderProgram>
#include "gtmaterial.h"
#include "../gtcamera.h"
#include "../gttexture2D.h"
#include "../gtrenderer.h"

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

void GtMaterialParameterBase::updateLocation(const QOpenGLShaderProgram* program)
{
    auto it = m_locations.insert(program, program->uniformLocation(m_name));
    if(*it == -1) {
        qCWarning(LC_SYSTEM) << "location not found" << m_name << "for shaders:";
        for(const auto* shader : program->shaders()) {
            qCWarning(LC_SYSTEM) << shader->sourceCode();
        }
    }
}

class GtRenderer* GtMaterialParameterBase::currentRenderer()
{
    return GtRenderer::currentRenderer();
}

void GtMaterialParameterBase::bind(QOpenGLShaderProgram* program, OpenGLFunctions* f)
{
    m_delegate(program, m_locations.value(program, 0), f);
}

void GtMaterialParameterBase::installDelegate()
{
    this->m_delegate = apply();
}

GtMaterialResourceParameterBase::GtMaterialResourceParameterBase(const QString& name, const Name& resource)
    : Super(name)
    , m_resource(resource)
{
}
