#include "gtmaterial.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

#include "gtmaterialparameterbase.h"
#include "../gtmeshbase.h"
#include "../gtrenderer.h"

GtMaterial::GtMaterial(gRenderType renderType)
    : m_renderType(renderType)
    , m_visible(true)
    , m_isValid(false)
{

}

GtMaterial::~GtMaterial()
{

}

void GtMaterial::AddParameter(const SharedPointer<GtMaterialParameterBase>& parameter)
{
    m_parameters.append(parameter);
}

void GtMaterial::AddMesh(const GtMeshPtr& mesh)
{
    m_meshs.append(mesh);
}

void GtMaterial::Draw(OpenGLFunctions* f)
{
    if(!m_visible || !m_isValid) {
        return;
    }

    Q_ASSERT(m_shaderProgram != nullptr);

    m_shaderProgram->bind();    
    updateParameters(f);

    for(const auto& mesh : m_meshs) {
        if(mesh->IsVisible())
            mesh->Draw(m_renderType, f);
    }

    m_shaderProgram->release();
}

void GtMaterial::SetVisible(bool visible)
{
    m_visible = visible;
}

void GtMaterial::SetRenderType(gRenderType renderType)
{
    m_renderType = renderType;
}

GtMaterial&GtMaterial::AddShader(GtMaterial::ShaderType type, const QString& file)
{
    m_shaders.Append(new Shader(file, type));
    return *this;
}

void GtMaterial::SetShaders(const QString& path, const QString& vert_file, const QString& frag_file)
{
    SetDir(path);
    AddShader(Vertex, vert_file);
    AddShader(Fragment, frag_file);
}

void GtMaterial::SetShaders(const QString& path, const QString& vertFile, const QString& geomFile, const QString& fragFile)
{
    SetDir(path);
    AddShader(Vertex, vertFile);
    AddShader(Geometry, geomFile);
    AddShader(Fragment, fragFile);
}

void GtMaterial::Update()
{
    m_shaderProgram.reset(new QOpenGLShaderProgram);
    m_shaderProgram->create();
    {
        if(m_shadersPath == ":/") {
            for(Shader* shader : m_shaders) {
                QOpenGLShader* shader_object = new QOpenGLShader((QOpenGLShader::ShaderTypeBit)shader->Type, m_shaderProgram.data());
                if(shader_object->compileSourceCode(extractShader(m_shadersPath + shader->File))) {
                    m_shaderProgram->addShader(shader_object);
                }
            }
        } else {
            DirBinder dir(m_shadersPath);
            m_shadersWatcher = new QtObserver(1000);
            auto* renderer = reinterpret_cast<GtRenderer*>(QThread::currentThread());
            for(Shader* shader : m_shaders) {
                QOpenGLShader* shader_object = new QOpenGLShader((QOpenGLShader::ShaderTypeBit)shader->Type, m_shaderProgram.data());
                m_shadersWatcher->AddFileObserver(m_shadersPath, shader->File, [this, renderer]{
                    renderer->Asynch([this]{
                        Update();
                    });
                });
                if(shader_object->compileSourceCode(extractShader(shader->File))) {
                    m_shaderProgram->addShader(shader_object);
                }
            }
        }
    }
    if(!m_shaderProgram->link()) {
        m_isValid = false;
        qCCritical(LC_SYSTEM) << "unable to link program" << m_shaderProgram->log();
    } else {
        m_isValid = true;
    }

    gTexUnit unit = 0;

    for(const auto& parameter : m_parameters) {
        parameter->updateLocation(m_shaderProgram.data());
        parameter->updateTextureUnit(unit);
        parameter->installDelegate();
    }
}

void GtMaterial::updateParameters(OpenGLFunctions* f)
{
    for(const auto& parameter : m_parameters) {
        parameter->bind(m_shaderProgram.data(), f);
    }
}

QByteArray GtMaterial::extractShader(const QString& fileName) const
{
    QFile file(fileName);
    if(file.open(QFile::ReadOnly)) {
        auto shader = file.readAll();
        return GtSharedShaderManager::GetInstance().Merge(shader);
    }
    return QByteArray();
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

    for(const auto& parameter : m_parameters) {
        parameter->MapProperties(observer);
    }
}
#include <QDirIterator>

void GtSharedShaderManager::Initialize(const QString& sharedShadersPath)
{
    Q_ASSERT(m_loadedShaders.isEmpty());
    QDirIterator it(sharedShadersPath, {"*.shader"});

    auto loadFile = [this](const QString& filePath, const QString& fileName) {
        QFile file(filePath);
        if(file.open(QFile::ReadOnly)) {
            m_loadedShaders.insert(Name(fileName), file.readAll());
        }
    };

    while (it.hasNext()) {
        it.next();
        auto filePath = it.filePath();
        auto fileName = it.fileName();
        loadFile(filePath, fileName);
        if(!filePath.startsWith(":")) {
            m_observer.AddFileObserver(filePath, [loadFile, filePath, fileName]{
                loadFile(filePath, fileName);
            });
        }
    }
}

const QByteArray& GtSharedShaderManager::Extract(const Name& fileName) const
{
    static QByteArray result;
    auto foundIt = m_loadedShaders.find(fileName);
    if(foundIt != m_loadedShaders.end()) {
        return foundIt.value();
    }
    return result;
}

QByteArray GtSharedShaderManager::Merge(const QByteArray& shader) const
{
    QByteArray result;
    QRegExp regExp(R"(#include \"([^\"]+)\")");
    qint32 pos = 0;
    qint32 lastPos = 0;
    while((pos = regExp.indexIn(shader, pos)) != -1) {
        result.append(shader.begin() + lastPos, pos - lastPos);
        result += Extract(Name(regExp.cap(1)));
        pos += regExp.matchedLength();
        lastPos = pos;
    }
    if(lastPos != 0) {
        result.append(shader.begin() + lastPos, std::distance(shader.begin(), shader.end()));
    } else {
        return shader;
    }
    return result;
}

GtSharedShaderManager& GtSharedShaderManager::GetInstance()
{
    static GtSharedShaderManager result;
    return result;
}
