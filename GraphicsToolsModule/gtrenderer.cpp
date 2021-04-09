#include "gtrenderer.h"

#include <QOpenGLFramebufferObject>

#include "GraphicsToolsModule/internal.hpp"
#include "GraphicsToolsModule/gtdepthbuffer.h"
#include "GraphicsToolsModule/gtplayercontrollercamera.h"

#include "GraphicsToolsModule/Objects/gtshaderprogram.h"
#include "gtrenderercontroller.h"

GtRendererSharedData::GtRendererSharedData(GtRenderer* base)
    : BaseRenderer(base)
{
}

GtRenderer::GtRenderer(GtRenderer* baseRenderer)
    : Super(baseRenderer->m_surfaceFormat, baseRenderer)
    , m_sharedData(baseRenderer->m_sharedData)
{
    construct();
}

void GtRenderer::CreateShaderProgramAlias(const Name& aliasName, const Name& sourceName)
{
    Q_ASSERT(m_sharedData->ShaderPrograms.contains(sourceName) && !m_sharedData->ShaderPrograms.contains(aliasName));
    m_sharedData->ShaderPrograms[aliasName] = m_sharedData->ShaderPrograms[sourceName];
}

void GtRenderer::construct()
{
    m_queueNumber = 0;
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

GtRenderer::GtRenderer(const QSurfaceFormat& format)
    : Super(format, nullptr)
    , m_sharedData(new GtRendererSharedData(this))
{   
    construct();
    CreateShaderProgram("DefaultTextShaderProgram")->SetShaders(GT_SHADERS_PATH, "sdftext.vert", "sdftext.geom", "sdftext.frag");
    CreateShaderProgram("DefaultText3DShaderProgram")->SetShaders(GT_SHADERS_PATH, "sdftext.vert", "sdftext3d.geom", "sdftext.frag");
    CreateShaderProgram("DefaultScreenTextShaderProgram")->SetShaders(GT_SHADERS_PATH, "sdfscreentext.vert", "sdfscreentext.geom", "sdftext.frag");
}

GtRenderer::~GtRenderer()
{
    OnAboutToBeDestroyed();
    Quit();
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
    m_sharedData->SharedResourcesSystem.RegisterResource(fontName, [this, fntFilePath, texturePath]{
        auto* result = new GtTexture2D(this);
        GtTextureFormat format;
        format.MagFilter = GL_LINEAR;
        format.MinFilter = GL_LINEAR;
        format.WrapS = GL_CLAMP_TO_EDGE;
        format.WrapT = GL_CLAMP_TO_EDGE;
        format.MipMapLevels = 0;
        result->SetFormat(format);
        result->LoadImg(texturePath);
        return result;
    });
}

void GtRenderer::CreateTexture(const Name& textureName, const std::function<GtTexture* (OpenGLFunctions*)>& textureLoader)
{
    m_sharedData->SharedResourcesSystem.RegisterResource(textureName, [this, textureLoader]{
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
    m_controllers.append(controller);
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

    m_resourceSystem.RegisterResource("mvp", []{
        return new Matrix4();
    });

    m_resourceSystem.RegisterResource("screenSize", []{
        return new Vector2F();
    });

    m_resourceSystem.RegisterResource("invertedMVP", []{
        return new Matrix4();
    });

    m_resourceSystem.RegisterResource("eye", []{
        return new Vector3F();
    });

    m_resourceSystem.RegisterResource("side", []{
        return new Vector3F();
    });

    m_resourceSystem.RegisterResource("up", []{
        return new Vector3F();
    });

    m_resourceSystem.RegisterResource("forward", []{
        return new Vector3F();
    });

    m_resourceSystem.RegisterResource("view", []{
        return new Matrix4;
    });

    m_resourceSystem.RegisterResource("projection", []{
        return new Matrix4;
    });

    m_resourceSystem.RegisterResource("rotation", []{
        return new Matrix4;
    });

    m_resourceSystem.RegisterResource("viewportProjection", []{
        return new Matrix4;
    });

    m_resourceSystem.RegisterResource("camera", []{
        return new GtCamera*;
    });

    m_scene = new GtScene();

    /*if(m_params->DebugMode) {
        auto* logger = new QOpenGLDebugLogger(this);
        if(logger->initialize()) {
            logger->startLogging();
        }
    }*/

    m_mvp = m_resourceSystem.GetResource<Matrix4>("mvp");
    m_eye = m_resourceSystem.GetResource<Vector3F>("eye");
    m_invertedMv = m_resourceSystem.GetResource<Matrix4>("invertedMVP");
    m_forward = m_resourceSystem.GetResource<Vector3F>("forward");
    m_up = m_resourceSystem.GetResource<Vector3F>("up");
    m_screenSize = m_resourceSystem.GetResource<Vector2F>("screenSize");
    m_view = m_resourceSystem.GetResource<Matrix4>("view");
    m_projection = m_resourceSystem.GetResource<Matrix4>("projection");
    m_rotation = m_resourceSystem.GetResource<Matrix4>("rotation");
    m_viewport = m_resourceSystem.GetResource<Matrix4>("viewportProjection");
    m_side = m_resourceSystem.GetResource<Vector3F>("side");
    m_camera = m_resourceSystem.GetResource<GtCamera*>("camera");

    // TODO. Must have state machine feather
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    glClearStencil(0x00);

    glDepthFunc(GL_LEQUAL);
    glLineWidth(5.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for(const auto& controller : m_controllers) {
        controller->onInitialize();
    }

    OnInitialized();

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
        m_delayedDraws.clear();
        auto* depthFbo = controller->m_depthFbo.get();
        auto* camera = controller->m_camera.get();
        auto cameraStateChanged = camera->IsFrameChangedReset();
        m_renderProperties = controller->m_renderProperties;
        m_renderProperties[RENDER_PROPERTY_CAMERA_STATE_CHANGED] = cameraStateChanged;

        controller->drawSpace(this);

        glViewport(0,0, fbo->width(), fbo->height());

        m_viewport->Data().Set(camera->GetViewportProjection());
        m_rotation->Data().Set(camera->GetRotation());
        m_projection->Data().Set(camera->GetProjection());
        m_view->Data().Set(camera->GetView());
        m_mvp->Data().Set(camera->GetWorld());
        m_eye->Data().Set(camera->GetEye());
        m_up->Data().Set(camera->GetUp());
        m_forward->Data().Set(camera->GetForward());
        m_invertedMv->Data().Set(camera->GetView().inverted().transposed());
        m_screenSize->Data().Set(Vector2F(fbo->size().width(), fbo->size().height()));
        m_side->Data().Set(Vector3F::crossProduct(m_up->Data().Get(), m_forward->Data().Get()).normalized());
        m_camera->Data().Set(camera);

        { // TODO. Fixing binding issues with shared resources
            QMutexLocker locker(&m_sharedData->Mutex);
            fbo->bind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            if(m_renderProperties.contains(RENDER_PROPERTY_FORCE_DISABLE_DEPTH_TEST)) {
                glDisable(GL_DEPTH_TEST);
            } else {
                glEnable(GL_DEPTH_TEST);
            }
            m_scene->draw(this);
            controller->draw(this);
            for(const auto& draws : m_delayedDraws) {
                draws();
            }

            fbo->release();

            depthFbo->Bind();

            glClear(GL_DEPTH_BUFFER_BIT);

            m_renderProperties[RENDER_PROPERTY_DRAWING_DEPTH_STAGE] = true;
            glEnable(GL_DEPTH_TEST);

            m_scene->drawDepth(this);
            controller->drawDepth(this);

            m_renderProperties[RENDER_PROPERTY_DRAWING_DEPTH_STAGE] = false;

            depthFbo->Release();

            auto* image = new QImage(fbo->toImage());
            controller->setCurrentImage(image, GetComputeTime());
        }       
    }


}

void GtRenderer::onDestroy()
{
    m_scene = nullptr;
    for(const auto& controller : m_controllers) {
        controller->onDestroy();
    }
}

