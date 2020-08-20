#include "gtmaterial.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include "gtmaterialparameterbase.h"
#include "../gtmeshbase.h"

GtMaterial::GtMaterial(gRenderType renderType)
    : m_renderType(renderType)
    , m_visible(true)
{

}

GtMaterial::~GtMaterial()
{

}

void GtMaterial::AddParameter(GtMaterialParameterBase* delegate)
{
    m_parameters.Push(delegate);
}

void GtMaterial::AddMesh(GtMeshBase* mesh)
{
    m_meshs.Append(mesh);
}

void GtMaterial::Draw(OpenGLFunctions* f)
{
    if(!m_visible) {
        return;
    }

    Q_ASSERT(m_shaderProgram != nullptr);
    m_shaderProgram->bind();

    for(GtMaterialParameterBase* parameter : m_parameters)
        parameter->bind(m_shaderProgram.data(), f);

    for(GtMeshBase* mesh : m_meshs) {
        if(mesh->IsVisible())
            mesh->Draw(m_renderType, f);
    }

    m_shaderProgram->release();
}

void GtMaterial::SetVisible(bool visible)
{
    m_visible = visible;
}

GtMaterial&GtMaterial::AddShader(GtMaterial::ShaderType type, const QString& file)
{
    m_shaders.Append(new Shader({file, type}));
    return *this;
}

void GtMaterial::SetShaders(const QString& path, const QString& vert_file, const QString& frag_file)
{
    SetDir(path);
    AddShader(Vertex, vert_file);
    AddShader(Fragment, frag_file);
    // Update();
}

void GtMaterial::SetShaders(const QString& path, const QString& vertFile, const QString& geomFile, const QString& fragFile)
{
    SetDir(path);
    AddShader(Vertex, vertFile);
    AddShader(Geometry, geomFile);
    AddShader(Fragment, fragFile);
    // Update();
}

void GtMaterial::Update()
{
    m_shaderProgram.reset(new QOpenGLShaderProgram);
    m_shaderProgram->create();
    {
        if(m_shadersPath == ":/") {
            for(Shader* shader : m_shaders) {
                QOpenGLShader* shader_object = new QOpenGLShader((QOpenGLShader::ShaderTypeBit)shader->Type, m_shaderProgram.data());
                if(shader_object->compileSourceFile(m_shadersPath + shader->File)) {
                    m_shaderProgram->addShader(shader_object);
                }
            }
        } else {
            DirBinder dir(m_shadersPath);
            for(Shader* shader : m_shaders) {
                QOpenGLShader* shader_object = new QOpenGLShader((QOpenGLShader::ShaderTypeBit)shader->Type, m_shaderProgram.data());
                if(shader_object->compileSourceFile(shader->File)) {
                    m_shaderProgram->addShader(shader_object);
                }
            }
        }
    }
    if(!m_shaderProgram->link()) {
        qCCritical(LC_SYSTEM) << "unable to link program" << m_shaderProgram->log();
    }

    gTexUnit unit = 0;

    for(GtMaterialParameterBase* parameter : m_parameters) {
        parameter->updateLocation(m_shaderProgram.data());
        parameter->updateTextureUnit(unit);
        parameter->installDelegate();
    }
}

void GtMaterial::UpdateParameters()
{
    gTexUnit unit = 0;

    for(GtMaterialParameterBase* parameter : m_parameters) {
        parameter->updateLocation(m_shaderProgram.data());
        parameter->updateTextureUnit(unit);
        parameter->installDelegate();
    }
}

void GtMaterial::MapProperties(QtObserver* observer)
{
    qint32 counter = 0;
    for(Shader* shader : m_shaders) {
        new ExternalTextFileNameProperty(Name("Shaders/" + QString::number(counter++)), shader->File);
        observer->AddFilePtrObserver(&m_shadersPath, &shader->File, [this]{
            this->Update();
        });
    }

    GtMaterialParameterBase::material() = this;

    for(GtMaterialParameterBase* parameter : m_parameters) {
        parameter->MapProperties(observer);
    }
}
