#include "gtmaterialparametervector3f.h"

#include <QOpenGLShaderProgram>
#include "ResourcesModule/resourcessystem.h"
#include "GraphicsToolsModule/gtrenderer.h"

GtMaterialParameterVector2F::GtMaterialParameterVector2F(const QString& name, const Name& resource)
    : Super(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterVector2F::apply()
{
    m_vector = currentRenderer()->GetResource<Vector2F>(m_resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, m_vector.Get());
    };
}


GtMaterialParameterVector3F::GtMaterialParameterVector3F(const QString& name, const Name& resource)
    : Super(name, resource)
{}

GtMaterialParameterBase::FDelegate GtMaterialParameterVector3F::apply()
{
    m_vector = currentRenderer()->GetResource<Vector3F>(m_resource);
    return  [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions*) {
        program->setUniformValue(loc, m_vector.Get());
    };
}

GtMaterialParameterVector2FArray::GtMaterialParameterVector2FArray(const QString& name, const Name& resource)
    : Super(name, resource)
{
}

GtMaterialParameterBase::FDelegate GtMaterialParameterVector2FArray::apply()
{
    m_gpuData = currentRenderer()->GetResource<QVector<Vector2F>>(m_resource);
    return [this](QOpenGLShaderProgram* program, gLocID loc, OpenGLFunctions* f) {
        const void* rawDataPtr = nullptr;
        int elementCount = 0;

        m_gpuData.GetAccess([&](const QVector<Vector2F>& arrayData) {
            rawDataPtr = arrayData.constData();
            elementCount = qMin(arrayData.size(), 32);
        });

        if (elementCount == 0 || rawDataPtr == nullptr) {
            return;
        }

        f->glUniform2fv(loc, elementCount, reinterpret_cast<const GLfloat*>(rawDataPtr));

        gLocID countLoc = program->uniformLocation(m_name + "_COUNT");
        if (countLoc != -1) {
            f->glUniform1i(countLoc, elementCount);
        }
    };
}
