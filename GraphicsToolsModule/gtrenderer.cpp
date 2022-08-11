#include "gtrenderer.h"

#include <QOpenGLFramebufferObject>

#include "GraphicsToolsModule/internal.hpp"
#include "GraphicsToolsModule/gtdepthbuffer.h"
#include "GraphicsToolsModule/gtplayercontrollercamera.h"

#include "GraphicsToolsModule/Objects/gtshaderprogram.h"
#include "gtrenderercontroller.h"
#include "gtrenderpath.h"

GtRendererSharedData::GtRendererSharedData(GtRenderer* base)
    : BaseRenderer(base)
{
}

GtRenderer::GtRenderer(GtRenderer* baseRenderer)
    : Super(baseRenderer)
    , m_sharedData(baseRenderer->m_sharedData)
    , m_updateRequested(true)
{
    construct();
}

void GtRenderer::UpdateFrame()
{
    m_updateRequested = true;
}

void GtRenderer::CreateShaderProgramAlias(const Name& aliasName, const Name& sourceName)
{
    Q_ASSERT(m_sharedData->ShaderPrograms.contains(sourceName) && !m_sharedData->ShaderPrograms.contains(aliasName));
    m_sharedData->ShaderPrograms[aliasName] = m_sharedData->ShaderPrograms[sourceName];
}

void GtRenderer::construct()
{
    m_queueNumber = 0;
    m_resourceSystem = ::make_shared<ResourcesSystem>();
    m_standardMeshs = new GtStandardMeshs();    
}

void GtRenderer::enableDepthTest()
{
    if(!m_renderProperties[RENDER_PROPERTY_FORCE_DISABLE_DEPTH_TEST].toBool() || m_renderProperties[RENDER_PROPERTY_DRAWING_DEPTH_STAGE].toBool()) {
        glEnable(GL_DEPTH_TEST);
    }
}

void GtRenderer::disableDepthTest()
{
    glDisable(GL_DEPTH_TEST);
}

void GtRenderer::addDelayedDraw(const FAction& drawAction)
{
    m_delayedDraws.append(drawAction);
}

GtRenderer::GtRenderer(const QString& defaultShadersPath)
    : Super(nullptr)
    , m_sharedData(new GtRendererSharedData(this))
{   
    construct();
    CreateShaderProgram("DefaultTextShaderProgram")->SetShaders(defaultShadersPath, "sdftext.vert", "sdftext.geom", "sdftext.frag");
    CreateShaderProgram("DefaultText3DShaderProgram")->SetShaders(defaultShadersPath, "sdftext.vert", "sdftext3d.geom", "sdftext.frag");
    CreateShaderProgram("DefaultScreenTextShaderProgram")->SetShaders(defaultShadersPath, "sdfscreentext.vert", "sdfscreentext.geom", "sdfscreentext.frag");
}

GtRenderer::~GtRenderer()
{
    OnAboutToBeDestroyed();
    Quit();
    auto resourceSystem = m_resourceSystem;
    ThreadsBase::DoMain([resourceSystem]{});
}

GtRendererControllerPtr GtRenderer::CreateDefaultController()
{
    auto* container = new ControllersContainer();
    new GtPlayerControllerCamera(Name("GtPlayerControllerCamera"), container);
    auto controller = ::make_shared<GtRendererController>(this, container, new GtControllersContext());
    AddController(controller);
    return controller;
}

void GtRenderer::LoadFont(const Name& fontName, const QString& fntFilePath, const QString& texturePath)
{
    Q_ASSERT(!m_sharedData->Fonts.contains(fontName));
    GtFontPtr font(new GtFont(fontName, fntFilePath));
    m_sharedData->Fonts.insert(fontName, font);
    CreateTexture(fontName, texturePath);
}

void GtRenderer::CreateTexture(const Name& textureName, const std::function<GtTexture* (OpenGLFunctions*)>& textureLoader)
{
    m_sharedData->SharedResourcesSystem.RegisterResource<GtTexture>(textureName, [this, textureLoader]{
        return textureLoader(this);
    });
}

void GtRenderer::CreateTexture(const Name& textureName, const QString& fileName, const GtTextureFormat& format)
{
    CreateTexture(textureName, [fileName, format](OpenGLFunctions* f) {
        auto* result = new GtTexture2D(f);
        result->SetFormat(format);
        result->LoadImg(fileName);
        return result;
    });
}

void GtRenderer::CreateTexture(const Name& textureName, const QString& fileName)
{
    GtTextureFormat format;
    format.MagFilter = GL_LINEAR;
    format.MinFilter = GL_LINEAR;
    format.WrapS = GL_CLAMP_TO_EDGE;
    format.WrapT = GL_CLAMP_TO_EDGE;
    format.MipMapLevels = 0;

    CreateTexture(textureName, fileName, format);
}

GtRenderer*& GtRenderer::currentRenderer()
{
    static thread_local GtRenderer* renderer = nullptr;
    return renderer;
}

void GtRenderer::CreateFontAlias(const Name& aliasName, const Name& sourceName)
{
    Q_ASSERT(m_sharedData->Fonts.contains(sourceName) && !m_sharedData->Fonts.contains(aliasName));
    m_sharedData->Fonts[aliasName] = m_sharedData->Fonts[sourceName];
}

const GtFontPtr& GtRenderer::GetFont(const Name& fontName) const
{
    Q_ASSERT(m_sharedData->Fonts.contains(fontName));
    return m_sharedData->Fonts[fontName];
}

void GtRenderer::AddController(const GtRendererControllerPtr& controller)
{
    if(IsRunning()) {
        Asynch([this, controller]{
            controller->onInitialize();
            m_controllers.insert(controller.get(), controller);
        });
    } else {
        m_controllers.insert(controller.get(), controller);
    }
}

void GtRenderer::RemoveController(GtRendererController* controller)
{
    if(IsRunning()) {
        Asynch([this, controller]{
            for(const auto& queue : controller->m_drawables) {
                for(auto* drawable : queue) {
                    drawable->Destroy();
                }
            }
            controller->m_renderPath = nullptr;
            m_controllers.remove(controller);
        });
        ProcessEvents();
    } else {
        m_controllers.remove(controller);
    }
}

void GtRenderer::AddDrawable(GtDrawableBase* drawable, qint32 queueNumber)
{
    Q_ASSERT(drawable->m_renderer == this);

    Asynch([this, drawable, queueNumber]{
        drawable->initialize(this);
        m_scene->AddDrawable(drawable, queueNumber);
    });
}

void GtRenderer::RemoveDrawable(GtDrawableBase* drawable)
{
    Asynch([this, drawable]{
        m_scene->RemoveDrawable(drawable);
    });
}

void GtRenderer::Update(const std::function<void (OpenGLFunctions*)>& handler)
{
    Asynch([this, handler]{
        handler(this);
    });
}

GtShaderProgramPtr GtRenderer::CreateShaderProgram(const Name& name)
{
    Q_ASSERT(m_sharedData->BaseRenderer == this);
    auto result = GtShaderProgramPtr(new GtShaderProgram(this));
    m_sharedData->ShaderPrograms.insert(name, result);
    return result;
}

GtShaderProgramPtr GtRenderer::GetShaderProgram(const Name& name) const
{
    auto foundIt = m_sharedData->ShaderPrograms.find(name);
    if(foundIt != m_sharedData->ShaderPrograms.end()) {
        return foundIt.value();
    }
    return nullptr;
}

/*Point3F GtRenderer::Project(const Point3F& position) const
{
    THREAD_ASSERT_IS_THREAD(this);
    auto result = m_controllersContext->Camera->Project(position);
    return result;
}*/

GtRendererPtr GtRenderer::CreateSharedRenderer()
{
    Q_ASSERT(!isRunning() && IsBaseRenderer());
    m_childRenderers.append(GtRendererPtr(new GtRenderer(this)));
    return m_childRenderers.last();
}

bool GtRenderer::onInitialize()
{
    if(!initializeOpenGLFunctions()) {
        qCInfo(LC_SYSTEM) << "Cannot initialize opengl functions";
        return false;
    }

    currentRenderer() = this;

    if(m_sharedData->BaseRenderer == this) {
        for(const auto& shaderProgram : m_sharedData->ShaderPrograms) {
            shaderProgram->Update();
        }
    }

    m_mvp = m_resourceSystem->RegisterResourceAndGet<Matrix4>(GtNames::mvp);
    m_screenSize = m_resourceSystem->RegisterResourceAndGet<Vector2F>(GtNames::screenSize);
    m_invertedMv = m_resourceSystem->RegisterResourceAndGet<Matrix4>(GtNames::invertedMVP);
    m_eye = m_resourceSystem->RegisterResourceAndGet<Vector3F>(GtNames::eye);
    m_side = m_resourceSystem->RegisterResourceAndGet<Vector3F>(GtNames::side);
    m_up = m_resourceSystem->RegisterResourceAndGet<Vector3F>(GtNames::up);
    m_forward = m_resourceSystem->RegisterResourceAndGet<Vector3F>(GtNames::forward);
    m_view = m_resourceSystem->RegisterResourceAndGet<Matrix4>(GtNames::view);
    m_projection = m_resourceSystem->RegisterResourceAndGet<Matrix4>(GtNames::projection);
    m_rotation = m_resourceSystem->RegisterResourceAndGet<Matrix4>(GtNames::rotation);
    m_viewport = m_resourceSystem->RegisterResourceAndGet<Matrix4>(GtNames::viewportProjection);
    m_camera = m_resourceSystem->RegisterResourceAndGet<GtCamera*>(GtNames::camera);

    m_scene = new GtScene();

    /*if(m_params->DebugMode) {
        auto* logger = new QOpenGLDebugLogger(this);
        if(logger->initialize()) {
            logger->startLogging();
        }
    }*/

    // TODO. Must have state machine feather
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    glClearStencil(0x00);
    glEnable(GL_POINT_SPRITE);

    glDepthFunc(GL_LEQUAL);
    glLineWidth(1.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_standardMeshs->initialize(this);

    for(const auto& controller : m_controllers) {
        controller->onInitialize();
    }

    OnInitialized.Resolve(true);

    return true;
}

SharedPointer<guards::LambdaGuard> GtRenderer::SetDefaultQueueNumber(qint32 queueNumber)
{
    auto old = m_queueNumber;
    m_queueNumber = queueNumber;
    return ::make_shared<guards::LambdaGuard>([this, old]{ m_queueNumber = old; });
}

ThreadHandler GtRenderer::CreateThreadHandler()
{
    return [this](const FAction& action) -> AsyncResult {
        if(QThread::currentThread() == this) {
            action();
            return AsyncSuccess();
        } else {
            return Asynch([action]{
                action();
            });
        }
    };
}

void GtRenderer::onDraw()
{
    if(!isInitialized()) {
        return;
    }

    for(const auto& controller : m_controllers) {
        controller->m_controllers->Input();
        auto* fbo = controller->m_fbo.get();
        if(fbo == nullptr || !controller->Enabled) {
            continue;
        }

        auto* camera = controller->m_camera.get();
        auto cameraStateChanged = camera->IsFrameChanged();

        if(!controller->isDirtyReset() && !m_updateRequested) {
            continue;
        }
        m_delayedDraws.clear();
        auto* depthFbo = controller->m_depthFbo.get();
        m_renderProperties = controller->m_renderProperties;
        m_renderProperties[RENDER_PROPERTY_CAMERA_STATE_CHANGED] = cameraStateChanged;

        controller->drawSpace(this);

        glViewport(0,0, fbo->width(), fbo->height());

        m_viewport = camera->GetViewportProjection();
        m_rotation = camera->GetRotation();
        m_projection = camera->GetProjection();
        m_view = camera->GetView();
        m_mvp = camera->GetWorld();
        m_eye = camera->GetEye();
        m_up = camera->GetUp();
        m_forward = camera->GetForward();
        m_invertedMv = camera->GetView().inverted().transposed();
        m_screenSize = Vector2F(fbo->size().width(), fbo->size().height());
        m_side = Vector3F::crossProduct(m_up.Get(), m_forward.Get()).normalized();
        m_camera = camera;

        { // TODO. Fixing binding issues with shared resources
            QMutexLocker locker(&m_sharedData->Mutex);
            fbo->bind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            if(m_renderProperties.contains(RENDER_PROPERTY_FORCE_DISABLE_DEPTH_TEST)) {
                glDisable(GL_DEPTH_TEST);
            } else {
                glEnable(GL_DEPTH_TEST);
            }
            controller->m_renderPath->Render(m_scene.get(), fbo->handle());
            //m_scene->DrawAll(this);
            controller->draw(this);
            for(const auto& draws : m_delayedDraws) {
                draws();
            }

            fbo->release();

            depthFbo->Bind();

            glClear(GL_DEPTH_BUFFER_BIT);

            m_renderProperties[RENDER_PROPERTY_DRAWING_DEPTH_STAGE] = true;
            glEnable(GL_DEPTH_TEST);

            m_scene->DrawDepth(this);
            controller->drawDepth(this);

            m_renderProperties[RENDER_PROPERTY_DRAWING_DEPTH_STAGE] = false;

            depthFbo->Release();

            auto* image = new QImage(fbo->toImage());
            controller->setCurrentImage(image, GetComputeTime());
        }       
    }

    m_updateRequested = false;
}

void GtRenderer::onDestroy()
{
    m_standardMeshs = nullptr;
    m_scene = nullptr;
    for(const auto& controller : m_controllers) {
        controller->onDestroy();
    }
}

IMPLEMENT_GLOBAL_NAME_1(RENDER_PROPERTY_CAMERA_STATE_CHANGED)
IMPLEMENT_GLOBAL_NAME_1(RENDER_PROPERTY_FORCE_DISABLE_DEPTH_TEST)
IMPLEMENT_GLOBAL_NAME_1(RENDER_PROPERTY_DRAWING_DEPTH_STAGE)
