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

void GtRenderer::construct()
{
    SpaceColor = "1e1e1e";
    m_queueNumber = 0;

    SpaceColor.Subscribe([this]{
        Asynch([this]{
            const auto& color = SpaceColor.Native();
            glClearColor(color.redF(), color.greenF(), color.blueF(), 1.f);
        });
    });
}

GtRenderer::GtRenderer(const QSurfaceFormat& format)
    : Super(format, nullptr)
    , m_sharedData(new GtRendererSharedData(this))
{   
    construct();
    auto textShaderProgram = CreateShaderProgram("DefaultTextShaderProgram");
    textShaderProgram->SetShaders(GT_SHADERS_PATH, "sdftext.vert", "sdftext.geom", "sdftext.frag");
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
        GtTextMap map;
        map.LoadFromFnt(fntFilePath);
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

GtRenderer*& GtRenderer::currentRenderer()
{
    static thread_local GtRenderer* renderer = nullptr;
    return renderer;
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

void GtRenderer::AddDrawable(GtDrawableBase* drawable)
{
    auto queueNumber = m_queueNumber;
    Asynch([this, drawable, queueNumber]{
        drawable->initialize(this);
        m_scene->AddDrawable(drawable, queueNumber);
    });
}

void GtRenderer::RemoveDrawable(GtDrawableBase* drawable)
{
    Asynch([this, drawable]{
        drawable->onDestroy(this);
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

void GtRenderer::onInitialize()
{
    if(!initializeOpenGLFunctions()) {
        qCInfo(LC_SYSTEM) << "Cannot initialize opengl functions";
        return;
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

    m_resourceSystem.RegisterResource("viewport", []{
        return new Matrix4;
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
    m_viewport = m_resourceSystem.GetResource<Matrix4>("viewport");

    // TODO. Must have state machine feather
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    glClearStencil(0x00);

    glDepthFunc(GL_LEQUAL);
    glLineWidth(5.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const auto& color = SpaceColor.Native();
    glClearColor(color.redF(), color.greenF(), color.blueF(), 1.f);


    for(const auto& controller : m_controllers) {
        controller->onInitialize();
    }

    OnInitialized();
}

SharedPointer<guards::LambdaGuard> GtRenderer::SetDefaultQueueNumber(qint32 queueNumber)
{
    auto old = m_queueNumber;
    m_queueNumber = queueNumber;
    return ::make_shared<guards::LambdaGuard>([this, old]{ m_queueNumber = old; });
}

void GtRenderer::onDraw()
{
    if(!isInitialized() /*|| !m_camera->IsFrameChangedReset()*/) {
        return;
    }

    for(const auto& controller : m_controllers) {
        controller->m_controllers->Input();
        auto* fbo = controller->m_fbo.get();
        if(fbo == nullptr || !controller->IsEnabled()) {
            continue;
        }
        auto* depthFbo = controller->m_depthFbo.get();
        auto* camera = controller->m_camera.get();

        glViewport(0,0, fbo->width(), fbo->height());

        m_viewport->Data().Set(camera->GetViewport());
        m_rotation->Data().Set(camera->GetRotation());
        m_projection->Data().Set(camera->GetProjection());
        m_view->Data().Set(camera->GetView());
        m_mvp->Data().Set(camera->GetWorld());
        m_eye->Data().Set(camera->GetEye());
        m_up->Data().Set(camera->GetUp());
        m_forward->Data().Set(camera->GetForward());
        m_invertedMv->Data().Set(camera->GetView().inverted().transposed());
        m_screenSize->Data().Set(Vector2F(fbo->size().width(), fbo->size().height()));

        fbo->bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        { // TODO. Fixing binding issues with shared resources
            QMutexLocker locker(&m_sharedData->Mutex);
            m_scene->draw(this);
        }

        fbo->release();

        depthFbo->Bind();

        glClear(GL_DEPTH_BUFFER_BIT);

        m_scene->drawDepth(this);

        depthFbo->Release();

        auto* image = new QImage(fbo->toImage());
        controller->setCurrentImage(image, GetComputeTime());
    }


}

void GtRenderer::onDestroy()
{
    m_scene = nullptr;
    for(const auto& controller : m_controllers) {
        controller->onDestroy();
    }
}

