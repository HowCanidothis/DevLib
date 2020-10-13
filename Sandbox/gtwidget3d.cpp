#include "gtwidget3d.h"

#include <QMouseEvent>
#include <QTimer>
#include <QOpenGLTexture>
#include <QLabel>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>

#include "GraphicsToolsModule/gtmeshquad2D.h"
#include "GraphicsToolsModule/gtmeshcircle2D.h"
#include "GraphicsToolsModule/gtmeshsurface.h"
#include "GraphicsToolsModule/Objects/gtmaterial.h"
#include "GraphicsToolsModule/gttexture2D.h"
#include "GraphicsToolsModule/gtframebufferobject.h"
#include "GraphicsToolsModule/gtshadowmaptechnique.h"
#include "GraphicsToolsModule/gtframetexture.h"
#include "GraphicsToolsModule/gtplayercontrollercamera.h"
#include "GraphicsToolsModule/gtcamera.h"
#include "GraphicsToolsModule/gtdepthbuffer.h"

#include "GraphicsToolsModule/Objects/gtmaterialparametermatrix.h"
#include "GraphicsToolsModule/Objects/gtmaterialparametershadow.h"
#include "GraphicsToolsModule/Objects/gtmaterialparametertexture.h"
#include "GraphicsToolsModule/Objects/gtmaterialparameterframetexture.h"

#include "ComputeGraphModule/computenodethreadsafe.h"
#include "ComputeGraphModule/computegraphbase.h"
#include "ComputeGraphModule/computenodevolcanorecognition.h"
#include "ComputeGraphModule/computegraphcore.h"
#include "ComputeGraphModule/inputframestream.h"

#include <opencv2/opencv.hpp>

#undef GT_SHADERS_PATH
#define GT_SHADERS_PATH "D:/Work/wps/libs/Content/Shaders"

GtWidget3D::GtWidget3D(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_controllers(new ControllersContainer())
    , m_controllersContext(new GtControllersContext())
    , camera(nullptr)
    , fps_board(nullptr)
    , lft_board(nullptr)
    , compute_board(nullptr)
    , fps_counter(new TimerClocks)
    , vulcans(nullptr)
    , logger(nullptr)
    , shadow_mapping(false)
{

    auto cameraController = new GtPlayerControllerCamera(Name("CameraController" + QString::number((size_t)this)), m_controllers.data());

    m_controllers->SetContext(m_controllersContext.data());

    QSurfaceFormat surface_format;
//    surface_format.setSamples(4);
    this->setFormat(surface_format);

    QTimer* framer = new QTimer(this);
    connect(framer, SIGNAL(timeout()), this, SLOT(update()));
    framer->start(20);

    setFocusPolicy(Qt::ClickFocus);
    setFocus();

    setMouseTracking(true);
}

GtWidget3D::~GtWidget3D()
{

}

void GtWidget3D::setName(const QString& name)
{
    output_node = new GtComputeNodeThreadSafe(name);
}

void GtWidget3D::setLogger(QOpenGLDebugLogger* logger)
{
    this->logger = logger;

    QSurfaceFormat format = this->format();
    format.setOption(QSurfaceFormat::DebugContext);
    setFormat(format);
}

void GtWidget3D::setShadowMapTechnique(bool flag)
{
    shadow_mapping = flag;
}

void GtWidget3D::setVolcans(ComputeNodeVolcanoRecognition* volcans)
{
    this->vulcans = volcans;
}

GtComputeNodeBase* GtWidget3D::getOutputNode() const
{
    return output_node;
}

void GtWidget3D::setCamera(GtCamera* camera)
{
    this->camera = camera;
    m_controllersContext->Camera = camera;
}

void GtWidget3D::initializeGL()
{
    if(!initializeOpenGLFunctions()) {
        qCCritical(LC_SYSTEM) << "initialize functions failed";
    }

    if(logger && logger->initialize()) {
        logger->startLogging();
    }
    ResourcesSystem::RegisterResource("output_texture", [this]{
         GtFrameTexture* result = new GtFrameTexture(this);
         result->createOutput();
         return result;
    });
    static_frame_texture = ResourcesSystem::GetResource<GtFrameTexture>("output_texture");

    ResourcesSystem::RegisterResource("shadow_map_technique",[this]{
        GtShadowMapTechnique* result = new GtShadowMapTechnique(this, SizeI(1024,1024));
        result->Create();
        return result;
    });
    ResourcesSystem::RegisterResource("sand_tex", [this]{
        GtTexture2D* result = new GtTexture2D(this);
        result->LoadImg("sand2");
        return result;
    });
    ResourcesSystem::RegisterResource("grass_tex", [this]{
        GtTexture2D* result = new GtTexture2D(this);
        result->LoadImg("grass2");
        return result;
    });
    ResourcesSystem::RegisterResource("mountain_tex", [this]{
        GtTexture2D* result = new GtTexture2D(this);
        result->LoadImg("mountain2");
        return result;
    });
    ResourcesSystem::RegisterResource("mvp", [this]{
        return new Matrix4();
    });
    ResourcesSystem::RegisterResource("mvp_shadow", [this]{
        return new Matrix4();
    });

    m_controllersContext->DepthBuffer = new GtDepthBuffer(this);
    static_frame_texture = ResourcesSystem::GetResource<GtFrameTexture>("output_texture");
    shadow_map_technique = ResourcesSystem::GetResource<GtShadowMapTechnique>("shadow_map_technique");
    MVP = ResourcesSystem::GetResource<Matrix4>("mvp");
    MVP_shadow = ResourcesSystem::GetResource<Matrix4>("mvp_shadow");

    surface_mesh = new GtMeshSurface(3000, 2400, 320);
    surface_mesh->Initialize(this);

    surface_material = new GtMaterial(GL_TRIANGLE_STRIP);
    surface_material->AddMesh(surface_mesh.data());

    surface_material->AddParameter(::make_shared<GtMaterialParameterTexture>("SandTex", "sand_tex"));
    surface_material->AddParameter(::make_shared<GtMaterialParameterTexture>("GrassTex", "grass_tex"));
    surface_material->AddParameter(::make_shared<GtMaterialParameterTexture>("MountainTex", "mountain_tex"));
    if(shadow_mapping) {
        surface_material->AddParameter(::make_shared<GtMaterialParameterShadow>("ShadowMap", "shadow_map_technique"));
    }

    surface_material->AddParameter(::make_shared<GtMaterialParameterMatrix>("MVP", "mvp"));
    surface_material->AddParameter(::make_shared<GtMaterialParameterMatrix>("ShadowMVP", "mvp_shadow"));
    surface_material->AddParameter(::make_shared<GtMaterialParameterFrameTexture>("HeightMap", "output_texture"));
    surface_material->AddParameter(::make_shared<GtMaterialParameterBase>("LightDirection", [this](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions*) {
      program->setUniformValue(loc, shadow_map_technique->Data().Get().GetCamera()->GetForward().normalized());
    }));

    surface_material->SetShaders(GT_SHADERS_PATH "/Depth", "sensor.vert", "sensor.frag");

    surface_material->Update();
    static bool added = false;
    if(!added) {
        surface_material->MapProperties(QtObserver::Instance());
        added = true;
    }


    if(shadow_mapping) {

        depth_material = new GtMaterial(GL_TRIANGLE_STRIP);
        depth_material->AddMesh(GtMeshQuad2D::Instance(this));
        gTexID texture = shadow_map_technique->Data().Get().GetDepthTexture();
        depth_material->AddParameter(::make_shared<GtMaterialParameterBase>("TextureMap", [texture](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions* f) {
            GtTexture2D::bindTexture(f, 0, texture);
            program->setUniformValue(loc, 0);
        }));
        depth_material->SetShaders(GT_SHADERS_PATH, "screen.vert", "screen.frag");
        depth_material->Update();
    }

    circle_mesh = new GtMeshCircle2D();
    circle_mesh->Initialize(this);

    color_material = new GtMaterial(GL_POINTS);
    color_material->AddMesh(circle_mesh.data());
    color_material->AddParameter(::make_shared<GtMaterialParameterMatrix>("MVP", "mvp"));
    color_material->AddParameter(::make_shared<GtMaterialParameterBase>("zValue", [](QOpenGLShaderProgram* program, quint32 loc, OpenGLFunctions*) {
        program->setUniformValue(loc, 400.0f);
    }));
    color_material->SetShaders(GT_SHADERS_PATH, "colored2d.vert", "colored.frag");
    color_material->Update();

    glPointSize(10.f);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_FRONT);

    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_DEPTH_TEST);
}

void GtWidget3D::resizeGL(int w, int h)
{
    Q_ASSERT(camera);

    GtFramebufferFormat fbo_format;
    fbo_format.SetDepthAttachment(GtFramebufferFormat::RenderBuffer);
    fbo_format.AddColorAttachment(GtFramebufferTextureFormat(GL_TEXTURE_2D, GL_RGBA8));
    auto fbo = new GtFramebufferObjectMultisampled(this, {w,h}, 4);
    fbo->Create(fbo_format);
    this->fbo.reset(fbo);

    GtFramebufferFormat depthFboFormat;
    depthFboFormat.SetDepthAttachment(GtFramebufferFormat::Texture);

    auto depthFbo = new GtFramebufferObject(this, {w,h});
    depthFbo->Create(depthFboFormat);
    m_depthFbo = depthFbo;

    m_controllersContext->DepthBuffer->SetFrameBuffer(depthFbo, context());

    camera->Resize(w,h);
}

static float getXFromCircleCoordinate(float x, const SizeF& ratio)
{
    return x * ratio.height();
}

static float getYFromCircleCoordinate(float y, const SizeF& ratio, float width)
{
    return width - y * ratio.width();
}

void GtWidget3D::paintGL()
{
    // __PERFORMANCE__

    m_controllers->Input();

    if(true) {
        fps_counter->Bind();
        {
            qint32 w, h;
            {
                MatGuard guard = output_node->GetThreadOutput();
                static_frame_texture->Data().Change().setInput(guard.GetOutput());
                static_frame_texture->Data().Change().update();
                w = guard.GetOutput()->rows;
                h = guard.GetOutput()->cols;
            }
            if(vulcans) {
                QMutexLocker locker(&vulcans->Mutex);
                SizeF ratio(float(surface_mesh->GetHeight()) / w, float(surface_mesh->GetWidth()) / h);

                circle_mesh->Resize(vulcans->Circles.size());
                auto it_mesh = circle_mesh->Begin();
                for(const cv::Vec3f& circle: vulcans->Circles) {
                    Circle2D* mesh_circle = *it_mesh;
                    float rx = getXFromCircleCoordinate(circle[0], ratio);
                    float ry = getYFromCircleCoordinate(circle[1], ratio, surface_mesh->GetHeight());
                    mesh_circle->Position = Point2F(rx, ry);
                    mesh_circle->Color = Color3F(1.f, 0.f, 1.f);
                    mesh_circle->Radius = Point2F(circle[2] * ratio.height(), circle[2] * ratio.width());
                    it_mesh++;
                }
                circle_mesh->Update();
            }
        }
        if(shadow_mapping) {
            shadow_map_technique->Data().Change().Bind({-11232.8f, -57.2747f, 10584.6f},{1766.52f, 1309.02f, 0.f});
            MVP->Data().Set(shadow_map_technique->Data().Change().GetWorldMatrix());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0,0,1024,1024);

            surface_material->Draw(this);
            color_material->Draw(this);
            shadow_map_technique->Data().Change().Release();

            static Matrix4 bias_matrix(
            0.5f, 0.0f, 0.0f, 0.5f,
            0.0f, 0.5f, 0.0f, 0.5f,
            0.0f, 0.0f, 0.5f, 0.5f,
            0.0f, 0.0f, 0.0f, 1.0f
            );
            Matrix4 shadow_MVP = bias_matrix * shadow_map_technique->Data().Change().GetWorldMatrix();

            fbo->Bind();
            MVP->Data().Set(camera->GetWorld());
            MVP_shadow->Data().Set(shadow_MVP);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glViewport(0,0,fbo->GetWidth(),fbo->GetHeight());

                surface_material->Draw(this);
                color_material->Draw(this);
            fbo->Release();

            m_depthFbo->Bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            glViewport(0,0,m_depthFbo->GetWidth(),m_depthFbo->GetHeight());

            surface_material->Draw(this);
            color_material->Draw(this);

            m_depthFbo->Release();
        }
        else {
            MVP->Data().Set(camera->GetWorld());

            fbo->Bind();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glViewport(0,0,fbo->GetWidth(),fbo->GetHeight());

                surface_material->Draw(this);
                color_material->Draw(this);
            fbo->Release();

            m_depthFbo->Bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            glViewport(0,0,m_depthFbo->GetWidth(),m_depthFbo->GetHeight());

            surface_material->Draw(this);
            color_material->Draw(this);

            m_depthFbo->Release();
        }

        qint64 frame_time = fps_counter->Release();
        if(lft_board) lft_board->setText(Nanosecs(frame_time).ToString("lft:"));
        if(fps_board) fps_board->setText("fps: " + QString::number(fps_counter->CalculateMeanValue().TimesPerSecond(), 'f', 10));
        if(compute_board) compute_board->setText("cps: " + QString::number(Nanosecs(ComputeGraphCore::Instance()->GetComputeTime()).TimesPerSecond(), 'f', 10));
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->GetID());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
    glBlitFramebuffer(0,0,fbo->GetWidth(),fbo->GetHeight(),0,0,this->width(), this->height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GtWidget3D::mouseMoveEvent(QMouseEvent* event)
{
    m_controllers->MouseMoveEvent(event);
}

void GtWidget3D::mousePressEvent(QMouseEvent* event)
{
    m_controllers->MousePressEvent(event);
}

void GtWidget3D::wheelEvent(QWheelEvent* event)
{
    m_controllers->WheelEvent(event);
}

void GtWidget3D::keyPressEvent(QKeyEvent *event)
{
    m_controllers->KeyPressEvent(event);
}

void GtWidget3D::keyReleaseEvent(QKeyEvent *event)
{
    m_controllers->KeyReleaseEvent(event);
}
