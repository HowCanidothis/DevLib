#ifndef GTVIEWBASE_H
#define GTVIEWBASE_H

#include "gtobjectbase.h"
#include "SharedModule/array.h"
#include "SharedModule/stack.h"

class GtMesh;
class GtCamera;
class GtMaterialParameterBase;

class GtSharedShaderManager
{
    GtSharedShaderManager()
        : m_observer(1000)
    {}

public:
    void Initialize(const QString& sharedShadersPath);
    const QByteArray& Extract(const Name& fileName) const;
    QByteArray Merge(const QByteArray& shader) const;
    static GtSharedShaderManager& GetInstance();

private:
    QHash<Name, QByteArray> m_loadedShaders;
    QtObserver m_observer;
};

class GtMaterial : public GtObjectBase
{
public:
    enum ShaderType {
        Vertex                 = 0x0001,
        Fragment               = 0x0002,
        Geometry               = 0x0004,
        TessellationControl    = 0x0008,
        TessellationEvaluation = 0x0010,
        Compute                = 0x0020
    };

    GtMaterial(gRenderType renderType);
    virtual ~GtMaterial();

    void AddParameter(const SharedPointer<GtMaterialParameterBase>&);
    void AddMesh(const SharedPointer<GtMesh>& mesh);
    void Draw(OpenGLFunctions* f);
    void SetVisible(bool visible);

    void SetRenderType(gRenderType renderType);
    void SetDir(const QString& path) { m_shadersPath = path; }
    GtMaterial& AddShader(ShaderType type, const QString& file);
    void SetShaders(const QString& path, const QString& vertFile, const QString& fragFile);
    void SetShaders(const QString& path, const QString& vertFile, const QString& geomFile, const QString& fragFile);

    void Update();

private:
    void updateParameters(OpenGLFunctions* f);
    QByteArray extractShader(const QString& fileName) const;

protected:
    friend class GtMaterialParameterBase;
    struct Shader {
        QString File;
        qint32 Type;

        Shader(const QString& file, qint32 type)
            : File(file)
            , Type(type)
        {}
    };

    DispatcherConnectionsSafe m_connections;
    QVector<SharedPointer<GtMaterialParameterBase>> m_parameters;
    QVector<SharedPointer<GtMesh>> m_meshs;
    ScopedPointer<class QOpenGLShaderProgram> m_shaderProgram;
    ArrayPointers<Shader> m_shaders;
    QString m_shadersPath;
    gRenderType m_renderType;
    ScopedPointer<QtObserver> m_shadersWatcher;
    bool m_visible;
    bool m_isValid;

    // GtObjectBase interface
public:
    void MapProperties(QtObserver* observer) Q_DECL_OVERRIDE;
};

#endif // GTVIEW_H
