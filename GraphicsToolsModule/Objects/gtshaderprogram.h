#ifndef GTSHADERPROGRAM_H
#define GTSHADERPROGRAM_H

#include <SharedGuiModule/internal.hpp>
#include <SharedModule/External/external.hpp>

class GtSharedShaderManager
{
    GtSharedShaderManager()
        : m_observer(1000, ThreadHandlerNoCheckMainLowPriority) // TODO. Main Thread?
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

class GtShaderProgram
{
    friend class GtRenderer;
    GtShaderProgram(class GtRenderer* renderer);
public:
    enum ShaderType {
        Vertex                 = 0x0001,
        Fragment               = 0x0002,
        Geometry               = 0x0004,
        TessellationControl    = 0x0008,
        TessellationEvaluation = 0x0010,
        Compute                = 0x0020
    };

    GtShaderProgram& AddShader(ShaderType type, const QString& file);
    void SetShaders(const QString& path, const QString& vertFile, const QString& fragFile);
    void SetShaders(const QString& path, const QString& vertFile, const QString& geomFile, const QString& fragFile);

    class QOpenGLShaderProgram* GetQOpenGLShaderProgram() { return m_shaderProgram.get(); }

    bool Bind();
    void Release();
    void Update();

    bool IsValid() const { return m_isValid; }

    Dispatcher OnUpdated;

private:
    QByteArray extractShader(const QString& fileName) const;

private:
    struct Shader {
        QString File;
        qint32 Type;

        Shader(const QString& file, qint32 type)
            : File(file)
            , Type(type)
        {}
    };

    GtRenderer* m_renderer;
    ScopedPointer<QOpenGLShaderProgram> m_shaderProgram;
    QString m_shadersPath;
    ArrayPointers<Shader> m_shaders;
    ScopedPointer<QtObserver> m_shadersWatcher;
    SharedPointer<QMutex> m_mutex;
    bool m_isValid;

};

using GtShaderProgramPtr = SharedPointer<GtShaderProgram>;

#endif // GTSHADERPROGRAM_H
