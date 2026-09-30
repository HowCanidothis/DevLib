#include "gtmaterialparameterbase.h"

#include <QOpenGLShaderProgram>
#include "gtshaderprogram.h"
#include "gtmaterial.h"
#include "../gtcamera.h"
#include "../gttexture2D.h"
#include "../gtrenderer.h"

GtMaterialParameterBase::GtMaterialParameterBase(const QString& name, const GtMaterialParameterBase::FDelegate& delegate)
    : m_delegate(delegate)
    , m_name(name)
{

}

GtMaterialParameterBase::GtMaterialParameterBase(const QString& name, const QVector<Vector2F>* array)
    : m_delegate([this, array](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions* f) {
        const void* rawDataPtr = array->constData();
        int elementCount = qMin(array->size(), 32);

        if (elementCount != 0 && rawDataPtr != nullptr) {
            f->glUniform2fv(loc, elementCount, reinterpret_cast<const GLfloat*>(rawDataPtr));
        }

        gLocID countLoc = program->uniformLocation(m_name + "_COUNT");
        if (countLoc != -1) {
            f->glUniform1i(countLoc, elementCount);
        }
    })
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

void GtMaterialParameterBase::updateLocation(const QOpenGLShaderProgram* program, const GtShaderProgram* gtProgram)
{
    auto it = m_locations.insert(program, program->uniformLocation(m_name));
    if(*it == -1 && m_required) {
        qCWarning(LC_SYSTEM) << "location not found" << m_name << "for shaders:" << gtProgram->ShadersPaths();
    }
}

void GtMaterialParameterBase::SetRequired(bool required)
{
    m_required = required;
}

class GtRenderer* GtMaterialParameterBase::currentRenderer()
{
    return GtRenderer::currentRenderer();
}

void GtMaterialParameterBase::bind(QOpenGLShaderProgram* program, OpenGLFunctions* f)
{
    auto location = m_locations.value(program, 0);
    if(location == -1) {
        return;
    }
    m_delegate(program, location, f);
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
