#ifndef GTRENDERER_H
#define GTRENDERER_H

#include <SharedGuiModule/internal.hpp>
#include <ResourcesModule/internal.hpp>

#include <GraphicsToolsModule/Objects/gtmaterialparameterbase.h>

#include "gtrendererbase.h"
#include "decl.h"

class GtRendererSharedData
{
public:
    GtRendererSharedData(class GtRenderer* base);

    QHash<Name, GtShaderProgramPtr> ShaderPrograms;
    GtRenderer* BaseRenderer;
    ResourcesSystem SharedResourcesSystem;
    QHash<Name, GtFontPtr> Fonts;
    QMutex Mutex;
};

DECLARE_GLOBAL_NAME(RENDER_PROPERTY_CAMERA_STATE_CHANGED)
DECLARE_GLOBAL_NAME(RENDER_PROPERTY_FORCE_DISABLE_DEPTH_TEST)
DECLARE_GLOBAL_NAME(RENDER_PROPERTY_DRAWING_DEPTH_STAGE)

class GtRenderer : public GtRendererBase, protected OpenGLFunctions
{
    Q_OBJECT
    using Super = GtRendererBase;
    GtRenderer(GtRenderer* baseRenderer);
public:    
    GtRenderer(const QString& defaultShadersPath = ":/");
    ~GtRenderer();

    GtRendererControllerPtr CreateDefaultController();

    void UpdateFrame();
    void CreateFontAlias(const Name& aliasName, const Name& sourceName);
    void LoadFont(const Name& fontName, const QString& fntFilePath, const QString& texturePath);
    void CreateTexture(const Name& textureName, const std::function<GtTexture* (OpenGLFunctions* f)>& textureLoader);
    void CreateTexture(const Name& textureName, const QString& fileName, const struct GtTextureFormat& format);
    void CreateTexture(const Name& textureName, const QString& fileName);
    const GtFontPtr& GetFont(const Name& fontName) const;
    void AddController(const GtRendererControllerPtr& controller);
    void RemoveController(GtRendererController* controller);

    SharedPointer<guards::LambdaGuard> SetDefaultQueueNumber(qint32 queueNumber);  

    void CreateShaderProgramAlias(const Name& aliasName, const Name& sourceName);
    GtShaderProgramPtr CreateShaderProgram(const Name& name);
    GtShaderProgramPtr GetShaderProgram(const Name& name) const;
    template<class T>
    TResource<T> GetResource(const Name& name)
    {
        auto resource = m_resourceSystem->GetResource<T>(name, true);
        if(resource != nullptr) {
            return resource;
        }
        return m_sharedData->SharedResourcesSystem.GetResource<T>(name);
    }

    GtRendererPtr CreateSharedRenderer();

    template<class T, typename ... Args>
    T* CreateDrawableQueued(qint32 queueNumber, Args... args)
    {
        auto* result = new T(this, args...);
        AddDrawable(result, queueNumber);
        return result;
    }
    template<class T, typename ... Args>
    T* CreateDrawable(Args... args)
    {
        return CreateDrawableQueued<T,Args...>(m_queueNumber, args...);
    }
    void AddDrawable(GtDrawableBase* drawable, qint32 queueNumber);
    void AddDrawable(GtDrawableBase* drawable) { AddDrawable(drawable, m_queueNumber); }
    void RemoveDrawable(GtDrawableBase* drawable);
    void Update(const std::function<void (OpenGLFunctions*)>& handler);
    const class GtStandardMeshs& GetStandardMeshs() const { return *m_standardMeshs; }

    ThreadHandler CreateThreadHandler();

    //Point3F Project(const Point3F& position) const;

    bool IsBaseRenderer() const { return m_sharedData->BaseRenderer == this; }

    AsyncResult OnInitialized;
    Dispatcher OnAboutToBeDestroyed;

private:
    void addDelayedDraw(const FAction& drawAction);
    void enableDepthTest();
    void disableDepthTest();
    void construct();
    static GtRenderer*& currentRenderer();
    QOpenGLContext* getContext() { return m_context.get(); }

    // GtRenderThread interface
protected:
    bool onInitialize() override;
    void onDraw() override;
    void onDestroy() override;

private:
    friend class GtMaterialParameterBase;
    friend class GtFont;
    friend class GtRendererController;
    friend class GtDrawableBase;
    friend class GtRenderPath;

    QHash<GtRendererController*, GtRendererControllerPtr> m_controllers;
    ScopedPointer<QImage> m_outputImage;
    Matrix4Resource m_mvp;
    Matrix4Resource m_view;
    Matrix4Resource m_projection;
    Matrix4Resource m_rotation;
    Matrix4Resource m_invertedMv;
    Matrix4Resource m_viewport;
    TResource<Vector3F> m_eye;
    TResource<Vector3F> m_forward;
    TResource<Vector3F> m_side;
    TResource<Vector3F> m_up;
    TResource<Vector2F> m_screenSize;
    TResource<GtCamera*> m_camera;
    ScopedPointer<GtStandardMeshs> m_standardMeshs;

    qint32 m_queueNumber;

    ScopedPointer<class GtScene> m_scene;

    SharedPointer<GtRendererSharedData> m_sharedData;
    SharedPointer<ResourcesSystem> m_resourceSystem;
    QVector<GtRendererPtr> m_childRenderers;
    GtRenderProperties m_renderProperties;
    QVector<FAction> m_delayedDraws;
    std::atomic_bool m_updateRequested;
};

#endif // GTRENDERER_H
