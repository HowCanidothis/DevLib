#include "openglwidget.h"
#include "GraphicsTools/gtcamera.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QMouseEvent>
#include <QTimer>

#include "mesh.h"
#include "fluid.h"

#include <NvFlex.h>
#include <NvFlexDevice.h>

#include "shadowmap.h"
#include "flex.h"
#include "convex.h"

#include "voxelize.h"
#include "sdf.h"

#include "trash.h"

const float kPi = 3.141592653589f;

#define glVerify(x) f->x

struct GpuMesh
{
    GLuint mPositionsVBO;
    GLuint mNormalsVBO;
    GLuint mIndicesIBO;

    int mNumVertices;
    int mNumFaces;

    static GpuMesh* CreateGpuMesh(OpenGLFunctions* f, const Mesh* m)
    {
        GpuMesh* mesh = new GpuMesh();

        mesh->mNumVertices = m->GetNumVertices();
        mesh->mNumFaces = m->GetNumFaces();

        // vbos
        glVerify(glGenBuffers(1, &mesh->mPositionsVBO));
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, mesh->mPositionsVBO));
        glVerify(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*m->m_positions.size(), &m->m_positions[0], GL_STATIC_DRAW));

        glVerify(glGenBuffers(1, &mesh->mNormalsVBO));
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, mesh->mNormalsVBO));
        glVerify(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*m->m_normals.size(), &m->m_normals[0], GL_STATIC_DRAW));

        glVerify(glGenBuffers(1, &mesh->mIndicesIBO));
        glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mIndicesIBO));
        glVerify(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*m->m_indices.size(), &m->m_indices[0], GL_STATIC_DRAW));
        glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        return mesh;
    }
};

// mapping of collision mesh to render mesh
std::map<NvFlexConvexMeshId, GpuMesh*> g_convexes;
std::map<NvFlexTriangleMeshId, GpuMesh*> g_meshes;
std::map<NvFlexDistanceFieldId, GpuMesh*> g_fields;

struct TrashConstants
{
    float dt = 1.0f / 60.0f;
    float waveTime = 0.0f;
    float waveFrequency = 1.5f;
    float waveAmplitude = 1.5f;
    float waveFloorTilt = 0.0f;
    float wavePlane;

    bool emitp = false;
    bool warmup = false;

    float windTime = 0.0f;
    float windStrength = 1.0f;

    float blur = 1.0f;
    Vector4F fluidColor = Vector4F(0.1f, 0.4f, 0.8f, 1.0f);
    bool drawEllipsoids = false;
    bool drawPoints = true;
    bool drawCloth = true;
    float expandCloth = 0.0f;

    bool drawOpaque = false;
    int drawSprings = false;
    bool drawDiffuse = false;
    bool drawMesh = true;
    bool drawRopes = true;
    bool drawDensity = false;
    float ior = 1.0f;
    float lightDistance = 2.0f;
    float fogDistance = 0.005f;

    float pointScale = 1.0f;
    float ropeScale = 1.0f;
    float drawPlaneBias = 0.0f;

    int numSubsteps = 2;

    float diffuseScale = 0.5f;
    Vector4F diffuseColor = Vector4F(1.f, 1.f, 1.f, 1.f);
    float diffuseMotionScale = 1.0f;
    bool diffuseShadow = false;
    float diffuseInscatter = 0.8f;
    float diffuseOutscatter = 0.53f;
    /* Note that this array of colors is altered by demo code, and is also read from global by graphics API impls */
    Color4F colors[8] =
    {
        Color4F(0.0f, 0.5f, 1.0f, 1.f),
        Color4F(0.797f, 0.354f, 0.000f, 1.f),
        Color4F(0.092f, 0.465f, 0.820f, 1.f),
        Color4F(0.000f, 0.349f, 0.173f, 1.f),
        Color4F(0.875f, 0.782f, 0.051f, 1.f),
        Color4F(0.000f, 0.170f, 0.453f, 1.f),
        Color4F(0.673f, 0.111f, 0.000f, 1.f),
        Color4F(0.612f, 0.194f, 0.394f, 1.f)
    };
    int numSolidParticles = 0;
    int mouseParticle = -1;

    int maxDiffuseParticles = 0;	// number of diffuse particles
    int maxNeighborsPerParticle = 96;
    int numExtraParticles = 0;	// number of particles allocated but not made active

    float sceneLower = FLT_MAX;
    float sceneUpper = -FLT_MAX;

    bool interop = true;
};

const char *vertexPointShader =
        "#version 450\n"
        "uniform mat4 MVP;"
        "layout(location = 0) in vec3 a_vertex;"
        "void main() {"
            "gl_Position = MVP * vec4(a_vertex, 1.0);"
        "}";

const char *fragmentPointShader =
        "#version 450\n"
        "out vec4 o_Color;"
        "void main() {"
            "o_Color = vec4(1.0,0.0,0.0,1.0);"
        "}";

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , camera(new GtCamera)
    , program(new QOpenGLShaderProgram)
    , vbo(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
    , vbo_indices(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer))
    , vao(new QOpenGLVertexArrayObject)
    , indices(0)
    , mesh(nullptr)
    , trash(new TrashConstants)
    , params(new NvFlexParams)
    , flexLib(nullptr)
{
    QSurfaceFormat format;
    format.setMajorVersion(4);
    format.setMinorVersion(5);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    setFormat(format);
    camera->setProjectionProperties(45,10.f,150000.f);

    QTimer* framer = new QTimer(this);
    connect(framer, SIGNAL(timeout()), this, SLOT(update()));
    framer->start(30);
}

OpenGLWidget::~OpenGLWidget()
{

}

bool g_Error = false;

void ErrorCallback(NvFlexErrorSeverity severity, const char* msg, const char* file, int line)
{
    printf("Flex: %s - %s:%d\n", msg, file, line);
    g_Error = (severity == eNvFlexLogError);
    //assert(0); asserts are bad for TeamCity
}


void OpenGLWidget::initializeGL()
{
    if(!initializeOpenGLFunctions()) {
        qWarning() << "Cannot initialze opengl";
        return;
    }

    f = this;

    flex_device = NvFlexDeviceGetSuggestedOrdinal();
    if(flex_device == -1) {
        qWarning() << "Unable to initialize nvidia flex";
        return;
    }

    bool success = NvFlexDeviceCreateCudaContext(flex_device);

    if (!success)
    {
        qWarning("Error creating CUDA context.\n");
        return;
    }

    NvFlexInitDesc desc;
    desc.deviceIndex = flex_device;
    desc.enableExtensions = true;
    desc.renderDevice = 0;
    desc.renderContext = 0;
    desc.computeContext = 0;
    desc.computeType = eNvFlexCUDA;

    flexLib = NvFlexInit(NV_FLEX_VERSION, ErrorCallback, &desc);

    if (g_Error || flexLib == NULL)
    {
        printf("Could not initialize Flex, exiting.\n");
        return;
    }

    qDebug() << "Compute Device: " << NvFlexGetDeviceName(flexLib);

    shadowMap = ShadowMap::ShadowCreate(this);



    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexPointShader);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentPointShader);
    if(!program->link()) {
        return;
    }

    ScopedPointer<Mesh> mesh = Mesh::ImportMesh(GT_SAMPLE_OBJECTS_PATH "banana.obj");
    if(mesh == nullptr) {
        qWarning() << "No mesh";
        return;
    }

    vbo->create();
    vbo->bind();
    vbo->allocate(mesh->m_positions.data(), mesh->GetPositionSize());
    vbo->release();

    vbo_indices->create();
    vbo_indices->bind();
    vbo_indices->allocate(mesh->m_indices.data(), mesh->GetIndicesSize());
    vbo_indices->release();

    indices = mesh->m_indices.size();

    vao->create();
    vao->bind();
        vbo->bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,false,0,0);
    vao->release();

    glPointSize(10.f);
    glClearColor(0.f,0.f,0.f,1.f);
}

void OpenGLWidget::resizeGL(int w, int h)
{
    camera->resize(w, h);
    glViewport(0,0,w,h);

    reshapeRender(w,h);

    if (fluidRenderer)
        fluidRenderer->DestroyFluidRenderer();
    fluidRenderer = FluidRenderer::CreateFluidRenderer(flexLib, this, w, h);

    static bool firstTime = true;
    if(firstTime) {
        initScene();
        firstTime = false;
    }
}

void OpenGLWidget::paintGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glPointSize(5.0f);

    glVerify(glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, msaaFbo));
    glVerify(glClearColor(0.f,0.f,0.f,0.f));
    glVerify(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    /*if(indices) {
        program->bind();
        QMatrix4x4 toScale = camera->getWorld();
        toScale.scale(100.f,100.f,100.f);
        program->setUniformValue("MVP", toScale);

        vao->bind();
        vbo_indices->bind();
        glDrawElements(GL_TRIANGLES,indices,GL_UNSIGNED_INT,0);
        vbo_indices->release();
        vao->release();

        program->release();
    }*/

    buffers->map();

    renderScene();

    buffers->unmap();


    if (msaaFbo)
    {
        // blit the msaa buffer to the window
        glVerify(glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, msaaFbo));
        glVerify(glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0));
        glVerify(glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_LINEAR));
    }

        // render help to back buffer
    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    glVerify(glClear(GL_DEPTH_BUFFER_BIT));


    NvFlexSetParticles(solver, buffers->positions.buffer, NULL);
    NvFlexSetVelocities(solver, buffers->velocities.buffer, NULL);
    NvFlexSetPhases(solver, buffers->phases.buffer, NULL);
    NvFlexSetActive(solver, buffers->activeIndices.buffer, NULL);

    NvFlexSetActiveCount(solver, buffers->activeIndices.size());

    // allow scene to update constraints etc
    //SyncScene();

    if (false)
    {
        NvFlexSetShapes(
            solver,
            buffers->shapeGeometry.buffer,
            buffers->shapePositions.buffer,
            buffers->shapeRotations.buffer,
            buffers->shapePrevPositions.buffer,
            buffers->shapePrevRotations.buffer,
            buffers->shapeFlags.buffer,
            int(buffers->shapeFlags.size()));

        //g_shapesChanged = false;
    }

    // tick solver
    NvFlexSetParams(solver, params.data());
    NvFlexUpdateSolver(solver, trash->dt, trash->numSubsteps, false);

    // read back base particle data
    // Note that flexGet calls don't wait for the GPU, they just queue a GPU copy
    // to be executed later.
    // When we're ready to read the fetched buffers we'll Map them, and that's when
    // the CPU will wait for the GPU flex update and GPU copy to finish.
    NvFlexGetParticles(solver, buffers->positions.buffer, NULL);
    NvFlexGetVelocities(solver, buffers->velocities.buffer, NULL);
    NvFlexGetNormals(solver, buffers->normals.buffer, NULL);

    // readback triangle normals
    if (buffers->triangles.size())
        NvFlexGetDynamicTriangles(solver, buffers->triangles.buffer, buffers->triangleNormals.buffer, buffers->triangles.size() / 3);

    // readback rigid transforms
    if (buffers->rigidOffsets.size())
        NvFlexGetRigids(solver, NULL, NULL, NULL, NULL, NULL, NULL, NULL, buffers->rigidRotations.buffer, buffers->rigidTranslations.buffer);

    // read back just the new diffuse particle count, render buffers will be updated during rendering
    NvFlexGetDiffuseParticles(solver, NULL, NULL, buffers->diffuseCount.buffer);
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    Point2I resolutional_screen_pos = resolutional(event->pos());
    if(event->buttons() == Qt::MiddleButton) {
        camera->rotate(last_screen_position - resolutional_screen_pos);
    }
    else if(event->buttons() == Qt::RightButton) {
        camera->rotateRPE(last_screen_position - resolutional_screen_pos);
    }
    else {
        Vector3F dist = last_plane_position - camera->unprojectPlane(resolutional_screen_pos);
        camera->translate(dist.x(), dist.y());
    }
    last_screen_position = resolutional_screen_pos;
    last_plane_position = camera->unprojectPlane(resolutional_screen_pos);
}

void OpenGLWidget::mousePressEvent(QMouseEvent* event)
{
    last_screen_position = resolutional(event->pos());
    last_plane_position = camera->unprojectPlane(last_screen_position);
    if(event->buttons() == Qt::MiddleButton) {
        camera->setRotationPoint(last_plane_position);
    }
}

QPoint OpenGLWidget::resolutional(const QPoint& p) const
{
    return p;
}

void OpenGLWidget::wheelEvent(QWheelEvent* event)
{
    camera->focusBind(event->pos());
    camera->zoom(event->delta() > 0);
    camera->focusRelease();
}

void OpenGLWidget::reshapeRender(qint32 w, qint32 h)
{
    if (format().samples())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (msaaFbo)
        {
            glDeleteFramebuffers(1, &msaaFbo);
            glDeleteRenderbuffers(1, &msaaColorBuf);
            glDeleteRenderbuffers(1, &msaaDepthBuf);
        }

        int samples;
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);

        // clamp samples to 4 to avoid problems with point sprite scaling
        samples = 4;

        glGenFramebuffers(1, &msaaFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFbo);

        glGenRenderbuffers(1, &msaaColorBuf);
        glBindRenderbuffer(GL_RENDERBUFFER, msaaColorBuf);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, w, h);

        glGenRenderbuffers(1, &msaaDepthBuf);
        glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthBuf);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msaaDepthBuf);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorBuf);

        glCheckFramebufferStatus(GL_FRAMEBUFFER);

        glEnable(GL_MULTISAMPLE);
    }
}

void OpenGLWidget::initScene()
{
    buffers = new SimBuffers(flexLib);

    buffers->map();

    buffers->positions.resize(0);
    buffers->velocities.resize(0);
    buffers->phases.resize(0);

    buffers->rigidOffsets.resize(0);
    buffers->rigidIndices.resize(0);
    buffers->rigidMeshSize.resize(0);
    buffers->rigidRotations.resize(0);
    buffers->rigidTranslations.resize(0);
    buffers->rigidCoefficients.resize(0);
    buffers->rigidPlasticThresholds.resize(0);
    buffers->rigidPlasticCreeps.resize(0);
    buffers->rigidLocalPositions.resize(0);
    buffers->rigidLocalNormals.resize(0);

    buffers->springIndices.resize(0);
    buffers->springLengths.resize(0);
    buffers->springStiffness.resize(0);
    buffers->triangles.resize(0);
    buffers->triangleNormals.resize(0);
    buffers->uvs.resize(0);

    meshSkinIndices.resize(0);
    meshSkinWeights.resize(0);

    emitters.resize(1);
    emitters[0].mEnabled = false;
    emitters[0].mSpeed = 1.0f;
    emitters[0].mLeftOver = 0.0f;
    emitters[0].mWidth = 8;

    buffers->shapeGeometry.resize(0);
    buffers->shapePositions.resize(0);
    buffers->shapeRotations.resize(0);
    buffers->shapePrevPositions.resize(0);
    buffers->shapePrevRotations.resize(0);
    buffers->shapeFlags.resize(0);

    ropes.resize(0);

    delete mesh; mesh = nullptr;

    trash = new TrashConstants();

    // sim params
    params->gravity[0] = 0.0f;
    params->gravity[1] = -9.8f;
    params->gravity[2] = 0.0f;

    params->wind[0] = 0.0f;
    params->wind[1] = 0.0f;
    params->wind[2] = 0.0f;

    params->radius = 0.15f;
    params->viscosity = 0.0f;
    params->dynamicFriction = 0.0f;
    params->staticFriction = 0.0f;
    params->particleFriction = 0.0f; // scale friction between particles by default
    params->freeSurfaceDrag = 0.0f;
    params->drag = 0.0f;
    params->lift = 0.0f;
    params->numIterations = 3;
    params->fluidRestDistance = 0.0f;
    params->solidRestDistance = 0.0f;

    params->anisotropyScale = 1.0f;
    params->anisotropyMin = 0.1f;
    params->anisotropyMax = 2.0f;
    params->smoothing = 1.0f;

    params->dissipation = 0.0f;
    params->damping = 0.0f;
    params->particleCollisionMargin = 0.0f;
    params->shapeCollisionMargin = 0.0f;
    params->collisionDistance = 0.0f;
    params->sleepThreshold = 0.0f;
    params->shockPropagation = 0.0f;
    params->restitution = 0.0f;

    params->maxSpeed = FLT_MAX;
    params->maxAcceleration = 100.0f;	// approximately 10x gravity

    params->relaxationMode = eNvFlexRelaxationLocal;
    params->relaxationFactor = 1.0f;
    params->solidPressure = 1.0f;
    params->adhesion = 0.0f;
    params->cohesion = 0.025f;
    params->surfaceTension = 0.0f;
    params->vorticityConfinement = 0.0f;
    params->buoyancy = 1.0f;
    params->diffuseThreshold = 100.0f;
    params->diffuseBuoyancy = 1.0f;
    params->diffuseDrag = 0.8f;
    params->diffuseBallistic = 16;
    params->diffuseLifetime = 2.0f;

    params->numPlanes = 1;

    NvFlexSolverDesc solverDesc;
    NvFlexSetSolverDescDefaults(&solverDesc);

    initFluidBlockScene();

    uint32_t numParticles = buffers->positions.size();
    uint32_t maxParticles = numParticles + trash->numExtraParticles;

    if (params->solidRestDistance == 0.0f)
        params->solidRestDistance = params->radius;

    // if fluid present then we assume solid particles have the same radius
    if (params->fluidRestDistance > 0.0f)
        params->solidRestDistance = params->fluidRestDistance;

    // set collision distance automatically based on rest distance if not alraedy set
    if (params->collisionDistance == 0.0f)
        params->collisionDistance = std::max(params->solidRestDistance, params->fluidRestDistance)*0.5f;

    // default particle friction to 10% of shape friction
    if (params->particleFriction == 0.0f)
        params->particleFriction = params->dynamicFriction*0.1f;

    // add a margin for detecting contacts between particles and shapes
    if (params->shapeCollisionMargin == 0.0f)
        params->shapeCollisionMargin = params->collisionDistance*0.5f;

    // calculate particle bounds
    Vector3F particleLower, particleUpper;
    GetParticleBounds(particleLower, particleUpper);

    // accommodate shapes
    Vector3F shapeLower, shapeUpper;
    GetShapeBounds(shapeLower, shapeUpper);

    // update bounds
    g_sceneLower = VMin(VMin(g_sceneLower, particleLower), shapeLower);
    g_sceneUpper = VMax(VMax(g_sceneUpper, particleUpper), shapeUpper);

    g_sceneLower -= Vector3F(params->collisionDistance, params->collisionDistance, params->collisionDistance);
    g_sceneUpper += Vector3F(params->collisionDistance, params->collisionDistance, params->collisionDistance);

    // update collision planes to match flexs
    Vector3F up = Vector3F(-trash->waveFloorTilt, 1.0f, 0.0f).normalized();

    (Vector4F&)params->planes[0] = Vector4F(up.x(), up.y(), up.z(), 0.0f);
    (Vector4F&)params->planes[1] = Vector4F(0.0f, 0.0f, 1.0f, -g_sceneLower.z());
    (Vector4F&)params->planes[2] = Vector4F(1.0f, 0.0f, 0.0f, -g_sceneLower.x());
    (Vector4F&)params->planes[3] = Vector4F(-1.0f, 0.0f, 0.0f, g_sceneUpper.x());
    (Vector4F&)params->planes[4] = Vector4F(0.0f, 0.0f, -1.0f, g_sceneUpper.z());
    (Vector4F&)params->planes[5] = Vector4F(0.0f, -1.0f, 0.0f, g_sceneUpper.y());

    trash->wavePlane = params->planes[2][3];

    buffers->diffusePositions.resize(trash->maxDiffuseParticles);
    buffers->diffuseVelocities.resize(trash->maxDiffuseParticles);
    buffers->diffuseCount.resize(1, 0);

    // for fluid rendering these are the Laplacian smoothed positions
    buffers->smoothPositions.resize(maxParticles);

    buffers->normals.resize(0);
    buffers->normals.resize(maxParticles);

    // initialize normals (just for rendering before simulation starts)
    int numTris = buffers->triangles.size() / 3;
    for (int i = 0; i < numTris; ++i)
    {
        Vector3F v0 = Vector3F(buffers->positions[buffers->triangles[i * 3 + 0]]);
        Vector3F v1 = Vector3F(buffers->positions[buffers->triangles[i * 3 + 1]]);
        Vector3F v2 = Vector3F(buffers->positions[buffers->triangles[i * 3 + 2]]);

        Vector3F n = Vector3F::crossProduct(v1 - v0, v2 - v0);

        buffers->normals[buffers->triangles[i * 3 + 0]] += Vector4F(n, 0.0f);
        buffers->normals[buffers->triangles[i * 3 + 1]] += Vector4F(n, 0.0f);
        buffers->normals[buffers->triangles[i * 3 + 2]] += Vector4F(n, 0.0f);
    }

    for (int i = 0; i < int(maxParticles); ++i)
        buffers->normals[i] = Vector4F(SafeNormalize(Vector3F(buffers->normals[i]), Vector3F(0.0f, 1.0f, 0.0f)), 0.0f);


    // save mesh positions for skinning
    if (this->mesh)
    {
        meshRestPositions = this->mesh->m_positions;
    }
    else
    {
        meshRestPositions.resize(0);
    }

    solverDesc.maxParticles = maxParticles;
    solverDesc.maxDiffuseParticles = trash->maxDiffuseParticles;
    solverDesc.maxNeighborsPerParticle = trash->maxNeighborsPerParticle;

    // main create method for the Flex solver
    solver = NvFlexCreateSolver(flexLib, &solverDesc);

    // create active indices (just a contiguous block for the demo)
    buffers->activeIndices.resize(buffers->positions.size());
    for (int i = 0; i < buffers->activeIndices.size(); ++i)
        buffers->activeIndices[i] = i;

    // resize particle buffers to fit
    buffers->positions.resize(maxParticles);
    buffers->velocities.resize(maxParticles);
    buffers->phases.resize(maxParticles);

    buffers->densities.resize(maxParticles);
    buffers->anisotropy1.resize(maxParticles);
    buffers->anisotropy2.resize(maxParticles);
    buffers->anisotropy3.resize(maxParticles);

    // save rest positions
    buffers->restPositions.resize(buffers->positions.size());
    for (int i = 0; i < buffers->positions.size(); ++i)
        buffers->restPositions[i] = buffers->positions[i];

    // builds rigids constraints
    if (buffers->rigidOffsets.size())
    {
        assert(buffers->rigidOffsets.size() > 1);

        const int numRigids = buffers->rigidOffsets.size() - 1;

        // If the centers of mass for the rigids are not yet computed, this is done here
        // (If the CreateParticleShape method is used instead of the NvFlexExt methods, the centers of mass will be calculated here)
        if (buffers->rigidTranslations.size() == 0)
        {
            buffers->rigidTranslations.resize(buffers->rigidOffsets.size() - 1, Vector3F());
            CalculateRigidCentersOfMass(&buffers->positions[0], buffers->positions.size(), &buffers->rigidOffsets[0], &buffers->rigidTranslations[0], &buffers->rigidIndices[0], numRigids);
        }

        // calculate local rest space positions
        buffers->rigidLocalPositions.resize(buffers->rigidOffsets.back());
        CalculateRigidLocalPositions(&buffers->positions[0], &buffers->rigidOffsets[0], &buffers->rigidTranslations[0], &buffers->rigidIndices[0], numRigids, &buffers->rigidLocalPositions[0]);

        // set rigidRotations to correct length, probably NULL up until here
        buffers->rigidRotations.resize(buffers->rigidOffsets.size() - 1, Quaternion());
    }

    // unmap so we can start transferring data to GPU
    buffers->unmap();

    //-----------------------------
    // Send data to Flex

    NvFlexCopyDesc copyDesc;
    copyDesc.dstOffset = 0;
    copyDesc.srcOffset = 0;
    copyDesc.elementCount = numParticles;

    NvFlexSetParams(solver, params.data());
    NvFlexSetParticles(solver, buffers->positions.buffer, &copyDesc);
    NvFlexSetVelocities(solver, buffers->velocities.buffer, &copyDesc);
    NvFlexSetNormals(solver, buffers->normals.buffer, &copyDesc);
    NvFlexSetPhases(solver, buffers->phases.buffer, &copyDesc);
    NvFlexSetRestParticles(solver, buffers->restPositions.buffer, &copyDesc);

    NvFlexSetActive(solver, buffers->activeIndices.buffer, &copyDesc);
    NvFlexSetActiveCount(solver, numParticles);

    // springs
    if (buffers->springIndices.size())
    {
        assert((buffers->springIndices.size() & 1) == 0);
        assert((buffers->springIndices.size() / 2) == buffers->springLengths.size());

        NvFlexSetSprings(solver, buffers->springIndices.buffer, buffers->springLengths.buffer, buffers->springStiffness.buffer, buffers->springLengths.size());
    }

    // rigids
    if (buffers->rigidOffsets.size())
    {
        NvFlexSetRigids(solver, buffers->rigidOffsets.buffer, buffers->rigidIndices.buffer, buffers->rigidLocalPositions.buffer, buffers->rigidLocalNormals.buffer, buffers->rigidCoefficients.buffer, buffers->rigidPlasticThresholds.buffer, buffers->rigidPlasticCreeps.buffer, buffers->rigidRotations.buffer, buffers->rigidTranslations.buffer, buffers->rigidOffsets.size() - 1, buffers->rigidIndices.size());
    }

    // inflatables
    if (buffers->inflatableTriOffsets.size())
    {
        NvFlexSetInflatables(solver, buffers->inflatableTriOffsets.buffer, buffers->inflatableTriCounts.buffer, buffers->inflatableVolumes.buffer, buffers->inflatablePressures.buffer, buffers->inflatableCoefficients.buffer, buffers->inflatableTriOffsets.size());
    }

    // dynamic triangles
    if (buffers->triangles.size())
    {
        NvFlexSetDynamicTriangles(solver, buffers->triangles.buffer, buffers->triangleNormals.buffer, buffers->triangles.size() / 3);
    }

    // collision shapes
    if (buffers->shapeFlags.size())
    {
        NvFlexSetShapes(
            solver,
            buffers->shapeGeometry.buffer,
            buffers->shapePositions.buffer,
            buffers->shapeRotations.buffer,
            buffers->shapePrevPositions.buffer,
            buffers->shapePrevRotations.buffer,
            buffers->shapeFlags.buffer,
            int(buffers->shapeFlags.size()));
    }

    // create render buffers
    fluidRenderBuffers = fluidRenderer->CreateFluidRenderBuffers(maxParticles, trash->interop);
    diffuseRenderBuffers = fluidRenderer->CreateDiffuseRenderBuffers(trash->maxDiffuseParticles, trash->interop);

    // perform initial sim warm up
    if (trash->warmup)
    {
        printf("Warming up sim..\n");

        // warm it up (relax positions to reach rest density without affecting velocity)
        NvFlexParams copy = *params;
        copy.numIterations = 4;

        NvFlexSetParams(solver, &copy);

        const int kWarmupIterations = 10;

        for (int i = 0; i < kWarmupIterations; ++i)
        {
            NvFlexUpdateSolver(solver, 0.0001f, 1, false);
            NvFlexSetVelocities(solver, buffers->velocities.buffer, NULL);
        }

        // udpate host copy
        NvFlexGetParticles(solver, buffers->positions.buffer, NULL);
        NvFlexGetSmoothParticles(solver, buffers->smoothPositions.buffer, NULL);
        NvFlexGetAnisotropy(solver, buffers->anisotropy1.buffer, buffers->anisotropy2.buffer, buffers->anisotropy3.buffer, NULL);

        printf("Finished warm up.\n");
    }
}

// TODO. Remove this trash
uint32_t seed1;
uint32_t seed2;

static void RandInit()
{
    seed1 = 315645664;
    seed2 = seed1 ^ 0x13ab45fe;
}

// random number generator
static uint32_t Rand()
{
    seed1 = ( seed2 ^ ( ( seed1 << 5 ) | ( seed1 >> 27 ) ) ) ^ ( seed1*seed2 );
    seed2 = seed1 ^ ( ( seed2 << 12 ) | ( seed2 >> 20 ) );

    return seed1;
}

// returns a random number in the range [min, max)
static uint32_t Rand(uint32_t min, uint32_t max)
{
    return min + Rand()%(max-min);
}

// returns random number between 0-1
static float Randf()
{
    uint32_t value = Rand();
    uint32_t limit = 0xffffffff;

    return ( float )value*( 1.0f/( float )limit );
}

// returns random number between min and max
static float Randf(float min, float max)
{
    //	return Lerp(min, max, ParticleRandf());
    float t = Randf();
    return (1.0f-t)*min + t*(max);
}

// returns random number between 0-max
static float Randf(float max)
{
    return Randf()*max;
}

// returns a random unit vector (also can add an offset to generate around an off axis vector)
static Vector3F RandomUnitVector()
{
    float phi = Randf(kPi*2.0f);
    float theta = Randf(kPi*2.0f);

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    return Vector3F(cosTheta*sinPhi,cosPhi,sinTheta*sinPhi);
}

void OpenGLWidget::initFluidBlockScene()
{
    float minSize = 0.5f;
    float maxSize = 0.7f;

    float radius = 0.1f;
    float restDistance = radius*0.55f;
    int group = 0;

    AddRandomConvex(6, Vector3F(5.0f, -0.1f, 0.6f), 1.0f, 1.0f, Vector3F(1.0f, 1.0f, 0.0f), 0.0f);

    float ly = 0.5f;

    AddRandomConvex(10, Vector3F(2.5f, ly*0.5f, 1.f), minSize*0.5f, maxSize*0.5f, Vector3F(0.0f, 1.0f, 0.0f), Randf(0.0f, 2.0f*kPi));

    AddRandomConvex(12, Vector3F(3.8f, ly-0.5f, 1.f), minSize, maxSize, Vector3F(1.0f, 0.0f, 0.0f), Randf(0.0f, 2.0f*kPi));
    AddRandomConvex(12, Vector3F(3.8f, ly-0.5f, 2.6f), minSize, maxSize, Vector3F(1.0f, 0.0f, 0.0f), 0.2f + Randf(0.0f, 2.0f*kPi));

    AddRandomConvex(12, Vector3F(4.6f, ly, 0.2f), minSize, maxSize, Vector3F(1.0f, 0.0f, 1.0f), Randf(0.0f, 2.0f*kPi));
    AddRandomConvex(12, Vector3F(4.6f, ly, 2.0f), minSize, maxSize, Vector3F(1.0f, 0.0f, 1.0f), 0.2f + Randf(0.0f, 2.0f*kPi));

    float size = 0.3f;
    for (int i=0; i < 1; ++i) {
        Mesh* mesh = Mesh::ImportMesh(GT_SAMPLE_OBJECTS_PATH "torus.obj");
        if (mesh)
            CreateParticleShape(mesh,
                                Vector3F(4.5f, 2.0f + radius*2.0f*i, 1.0f),
                                Vector3F(size,size,size),
                                0.0f,
                                radius*0.5f,
                                Vector3F(),
                                0.125f,
                                true,
                                1.0f,
                                NvFlexMakePhase(group++, 0),
                                true,
                                0.0f);

        delete mesh;
    }

    trash->numSolidParticles = buffers->positions.size();

    float sizex = 1.76f;
    float sizey = 2.20f;
    float sizez = 3.50f;

    int x = int(sizex/restDistance);
    int y = int(sizey/restDistance);
    int z = int(sizez/restDistance);

    CreateParticleGrid(Vector3F(0.0f, restDistance*0.5f, 0.0f),
                       x,
                       y,
                       z,
                       restDistance,
                       Vector3F(),
                       1.0f,
                       false,
                       0.0f,
                       NvFlexMakePhase(group++, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid));

    params->radius = radius;
    params->dynamicFriction = 0.0f;
    params->viscosity = 0.0f;
    params->numIterations = 3;
    params->vorticityConfinement = 40.f;
    params->anisotropyScale = 20.0f;
    params->fluidRestDistance = restDistance;
    params->numPlanes = 5;
    //params->cohesion = 0.05f;

    trash->maxDiffuseParticles = 128*1024;
    trash->diffuseScale = 0.75f;

    trash->waveFloorTilt = -0.025f;

    trash->lightDistance *= 0.5f;

    // draw options
    trash->drawDensity = true;
    trash->drawDiffuse = true;
    trash->drawEllipsoids = true;
    trash->drawPoints = false;

    trash->warmup = true;
}

void OpenGLWidget::AddRandomConvex(int numPlanes, const Vector3F& position, float minDist, float maxDist, const Vector3F& axis, float angle)
{
    const int maxPlanes = 12;

    // 12-kdop
    const Vector3F directions[maxPlanes] = {
        Vector3F(1.0f, 0.0f, 0.0f),
        Vector3F(0.0f, 1.0f, 0.0f),
        Vector3F(0.0f, 0.0f, 1.0f),
        Vector3F(-1.0f, 0.0f, 0.0f),
        Vector3F(0.0f, -1.0f, 0.0f),
        Vector3F(0.0f, 0.0f, -1.0f),
        Vector3F(1.0f, 1.0f, 0.0f),
        Vector3F(-1.0f, -1.0f, 0.0f),
        Vector3F(1.0f, 0.0f, 1.0f),
        Vector3F(-1.0f, 0.0f, -1.0f),
        Vector3F(0.0f, 1.0f, 1.0f),
        Vector3F(0.0f, -1.0f, -1.0f),
    };

    numPlanes = clamp(6, numPlanes, maxPlanes);

    int mesh = NvFlexCreateConvexMesh(flexLib);

    NvFlexVector<Vector4F> planes(flexLib);
    planes.map();

    // create a box
    for (int i=0; i < numPlanes; ++i)
    {
        Vector4F plane = Vector4F(directions[i].normalized(), -Randf(minDist, maxDist));
        planes.push_back(plane);
    }

    buffers->shapePositions.push_back(Vector4F(position.x(), position.y(), position.z(), 0.0f));
    buffers->shapeRotations.push_back(Quaternion::fromAxisAndAngle(axis, angle));

    buffers->shapePrevPositions.push_back(buffers->shapePositions.back());
    buffers->shapePrevRotations.push_back(buffers->shapeRotations.back());

    // set aabbs
    ConvexMeshBuilder builder(&planes[0]);
    builder(numPlanes);

    Vector3F lower(FLT_MAX, FLT_MAX, FLT_MAX), upper(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (size_t v=0; v < builder.mVertices.size(); ++v)
    {
        const Vector3F p =  builder.mVertices[v];

        lower = VMin(lower, p);
        upper = VMax(upper, p);
    }

    planes.unmap();

    NvFlexUpdateConvexMesh(flexLib, mesh, planes.buffer, planes.size(), &lower[0], &upper[0]);

    NvFlexCollisionGeometry geo;
    geo.convexMesh.mesh = mesh;
    geo.convexMesh.scale[0] = 1.0f;
    geo.convexMesh.scale[1] = 1.0f;
    geo.convexMesh.scale[2] = 1.0f;

    buffers->shapeGeometry.push_back(geo);

    int flags = NvFlexMakeShapeFlags(eNvFlexShapeConvexMesh, false);
    buffers->shapeFlags.push_back(flags);


    // create render mesh for convex
    Mesh renderMesh;

    for (uint32_t j = 0; j < builder.mIndices.size(); j += 3)
    {
        uint32_t a = builder.mIndices[j + 0];
        uint32_t b = builder.mIndices[j + 1];
        uint32_t c = builder.mIndices[j + 2];

        Vector3F n = (Vector3F::crossProduct(builder.mVertices[b] - builder.mVertices[a], builder.mVertices[c] - builder.mVertices[a])).normalized();

        int startIndex = renderMesh.m_positions.size();

        renderMesh.m_positions.push_back(Point3F(builder.mVertices[a]));
        renderMesh.m_normals.push_back(n);

        renderMesh.m_positions.push_back(Point3F(builder.mVertices[b]));
        renderMesh.m_normals.push_back(n);

        renderMesh.m_positions.push_back(Point3F(builder.mVertices[c]));
        renderMesh.m_normals.push_back(n);

        renderMesh.m_indices.push_back(startIndex+0);
        renderMesh.m_indices.push_back(startIndex+1);
        renderMesh.m_indices.push_back(startIndex+2);
    }

    // insert into the global mesh list
    GpuMesh* gpuMesh = GpuMesh::CreateGpuMesh(this, &renderMesh);
    g_convexes[mesh] = gpuMesh;
}

void OpenGLWidget::CreateParticleShape(const Mesh* srcMesh, const Vector3F& lower, const Vector3F& scale, float rotation, float spacing, const Vector3F& velocity, float invMass, bool rigid, float rigidStiffness, int phase, bool skin, float jitter, const Vector3F& skinOffset, float skinExpand, const Vector4F& color, float springStiffness)
{
    if (rigid && buffers->rigidIndices.empty())
        buffers->rigidOffsets.push_back(0);

    if (!srcMesh)
        return;

    // duplicate mesh
    Mesh mesh;
    mesh.AddMesh(*srcMesh);

    int startIndex = int(buffers->positions.size());

    {
        mesh.Rotate(rotation, Vector3F(0.0f, 1.0f, 0.0f));

        Vector3F meshLower, meshUpper;
        mesh.GetBounds(meshLower, meshUpper);

        Vector3F edges = meshUpper-meshLower;
        float maxEdge = std::max(std::max(edges.x(), edges.y()), edges.z());

        // put mesh at the origin and scale to specified size
        Matrix4 scaleMatrix;
        scaleMatrix.scale(scale/maxEdge);
        Matrix4 translationMatrix;
        translationMatrix.translate(-meshLower);
        Matrix4 xform = scaleMatrix * translationMatrix;

        mesh.Transform(xform);
        mesh.GetBounds(meshLower, meshUpper);

        // recompute expanded edges
        edges = meshUpper-meshLower;
        maxEdge = std::max(std::max(edges.x(), edges.y()), edges.z());

        // tweak spacing to avoid edge cases for particles laying on the boundary
        // just covers the case where an edge is a whole multiple of the spacing.
        float spacingEps = spacing*(1.0f - 1e-4f);

        // make sure to have at least one particle in each dimension
        int dx, dy, dz;
        dx = spacing > edges.x() ? 1 : int(edges.x()/spacingEps);
        dy = spacing > edges.y() ? 1 : int(edges.y()/spacingEps);
        dz = spacing > edges.z() ? 1 : int(edges.z()/spacingEps);

        int maxDim = std::max(std::max(dx, dy), dz);

        // expand border by two voxels to ensure adequate sampling at edges
        meshLower -= 2.0f*Vector3F(spacing, spacing, spacing);
        meshUpper += 2.0f*Vector3F(spacing, spacing, spacing);
        maxDim += 4;

        std::vector<uint32_t> voxels(maxDim*maxDim*maxDim);

        // we shift the voxelization bounds so that the voxel centers
        // lie symmetrically to the center of the object. this reduces the
        // chance of missing features, and also better aligns the particles
        // with the mesh
        Vector3F meshOffset;
        meshOffset[0] = 0.5f * (spacing - (edges.x() - (dx-1)*spacing));
        meshOffset[1] = 0.5f * (spacing - (edges.y() - (dy-1)*spacing));
        meshOffset[2] = 0.5f * (spacing - (edges.z() - (dz-1)*spacing));
        meshLower -= meshOffset;

        //Voxelize(*mesh, dx, dy, dz, &voxels[0], meshLower - Vec3(spacing*0.05f) , meshLower + Vec3(maxDim*spacing) + Vec3(spacing*0.05f));
        Voxelize((const Vector3F*)&mesh.m_positions[0],
                mesh.m_positions.size(),
                (const int*)&mesh.m_indices[0],
                mesh.m_indices.size(),
                maxDim,
                maxDim,
                maxDim,
                &voxels[0],
                meshLower,
                meshLower + Vector3F(maxDim*spacing, maxDim*spacing, maxDim*spacing));

        std::vector<int> indices(maxDim*maxDim*maxDim);
        std::vector<float> sdf(maxDim*maxDim*maxDim);
        MakeSDF(&voxels[0], maxDim, maxDim, maxDim, &sdf[0]);

        for (int x=0; x < maxDim; ++x)
        {
            for (int y=0; y < maxDim; ++y)
            {
                for (int z=0; z < maxDim; ++z)
                {
                    const int index = z*maxDim*maxDim + y*maxDim + x;

                    // if voxel is marked as occupied the add a particle
                    if (voxels[index])
                    {
                        if (rigid)
                            buffers->rigidIndices.push_back(int(buffers->positions.size()));

                        Vector3F position = lower + meshLower + spacing*Vector3F(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f) + RandomUnitVector()*jitter;

                        // normalize the sdf value and transform to world scale
                        Vector3F n = SafeNormalize(SampleSDFGrad(&sdf[0], maxDim, x, y, z));
                        float d = sdf[index]*maxEdge;

                        if (rigid)
                            buffers->rigidLocalNormals.push_back(Vector4F(n, d));

                        // track which particles are in which cells
                        indices[index] = buffers->positions.size();

                        buffers->positions.push_back(Vector4F(position.x(), position.y(), position.z(), invMass));
                        buffers->velocities.push_back(velocity);
                        buffers->phases.push_back(phase);
                    }
                }
            }
        }
        scaleMatrix.setToIdentity();
        translationMatrix.setToIdentity();
        scaleMatrix.scale(1.0f + skinExpand);
        translationMatrix.translate(-0.5f*(meshUpper+meshLower));

        mesh.Transform(scaleMatrix * translationMatrix);

        translationMatrix.setToIdentity();
        translationMatrix.translate(lower + 0.5f*(meshUpper+meshLower));

        mesh.Transform(translationMatrix);


        if (springStiffness > 0.0f)
        {
            // construct cross link springs to occupied cells
            for (int x=0; x < maxDim; ++x)
            {
                for (int y=0; y < maxDim; ++y)
                {
                    for (int z=0; z < maxDim; ++z)
                    {
                        const int centerCell = z*maxDim*maxDim + y*maxDim + x;

                        // if voxel is marked as occupied the add a particle
                        if (voxels[centerCell])
                        {
                            const int width = 1;

                            // create springs to all the neighbors within the width
                            for (int i=x-width; i <= x+width; ++i)
                            {
                                for (int j=y-width; j <= y+width; ++j)
                                {
                                    for (int k=z-width; k <= z+width; ++k)
                                    {
                                        const int neighborCell = k*maxDim*maxDim + j*maxDim + i;

                                        if (neighborCell > 0 && neighborCell < int(voxels.size()) && voxels[neighborCell] && neighborCell != centerCell)
                                        {
                                            CreateSpring(indices[neighborCell], indices[centerCell], springStiffness);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }


    if (skin)
    {
        buffers->rigidMeshSize.push_back(mesh.GetNumVertices());

        int startVertex = 0;

        if (!this->mesh)
            this->mesh = new Mesh();

        // append to mesh
        startVertex = this->mesh->GetNumVertices();

        Matrix4 translationMatrix;
        translationMatrix.translate(skinOffset);
        this->mesh->Transform(translationMatrix);
        this->mesh->AddMesh(mesh);

        const Color3F colors[7] =
        {
            Color3F(0.0f, 0.5f, 1.0f),
            Color3F(0.797f, 0.354f, 0.000f),
            Color3F(0.000f, 0.349f, 0.173f),
            Color3F(0.875f, 0.782f, 0.051f),
            Color3F(0.01f, 0.170f, 0.453f),
            Color3F(0.673f, 0.111f, 0.000f),
            Color3F(0.612f, 0.194f, 0.394f)
        };

        for (uint32_t i=startVertex; i < this->mesh->GetNumVertices(); ++i)
        {
            int indices[4] = { -1, -1, -1, -1 };
            float distances[4] = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

            if (color.lengthSquared() == 0.0f)
                this->mesh->m_colours[i] = colors[((unsigned int)(phase))%7]*1.25f;
            else
                this->mesh->m_colours[i] = color.toVector3D();

            // find closest n particles
            for (int j=startIndex; j < buffers->positions.size(); ++j)
            {
                Vector3F v = Vector3F(this->mesh->m_positions[i])-Vector3F(buffers->positions[j]);
                float dSq = v.lengthSquared();

                // insertion sort
                int w=0;
                for (; w < 4; ++w)
                    if (dSq < distances[w])
                        break;

                if (w < 4)
                {
                    // shuffle down
                    for (int s=3; s > w; --s)
                    {
                        indices[s] = indices[s-1];
                        distances[s] = distances[s-1];
                    }

                    distances[w] = dSq;
                    indices[w] = int(j);
                }
            }

            // weight particles according to distance
            float wSum = 0.0f;

            for (int w=0; w < 4; ++w)
            {
                // convert to inverse distance
                distances[w] = 1.0f/(0.1f + powf(distances[w], .125f));

                wSum += distances[w];

            }

            float weights[4];
            for (int w=0; w < 4; ++w)
                weights[w] = distances[w]/wSum;

            for (int j=0; j < 4; ++j)
            {
                meshSkinIndices.push_back(indices[j]);
                meshSkinWeights.push_back(weights[j]);
            }
        }
    }

    if (rigid)
    {
        buffers->rigidCoefficients.push_back(rigidStiffness);
        buffers->rigidOffsets.push_back(int(buffers->rigidIndices.size()));
    }
}

void OpenGLWidget::CreateParticleGrid(const Vector3F& lower, int dimx, int dimy, int dimz, float radius, const Vector3F& velocity, float invMass, bool rigid, float rigidStiffness, int phase, float jitter)
{
    if (rigid && buffers->rigidIndices.empty())
        buffers->rigidOffsets.push_back(0);

    for (int x = 0; x < dimx; ++x)
    {
        for (int y = 0; y < dimy; ++y)
        {
            for (int z=0; z < dimz; ++z)
            {
                if (rigid)
                    buffers->rigidIndices.push_back(int(buffers->positions.size()));

                Vector3F position = lower + Vector3F(float(x), float(y), float(z))*radius + RandomUnitVector()*jitter;

                buffers->positions.push_back(Vector4F(position.x(), position.y(), position.z(), invMass));
                buffers->velocities.push_back(velocity);
                buffers->phases.push_back(phase);
            }
        }
    }

    if (rigid)
    {
        buffers->rigidCoefficients.push_back(rigidStiffness);
        buffers->rigidOffsets.push_back(int(buffers->rigidIndices.size()));
    }
}

void OpenGLWidget::CreateSpring(int i, int j, float stiffness, float give)
{
    buffers->springIndices.push_back(i);
    buffers->springIndices.push_back(j);
    buffers->springLengths.push_back((1.0f+give)*(buffers->positions[i].toVector3D()-buffers->positions[j].toVector3D()).length());
    buffers->springStiffness.push_back(stiffness);
}

void OpenGLWidget::GetParticleBounds(Vector3F& lower, Vector3F& upper)
{
    lower = Vector3F(FLT_MAX, FLT_MAX, FLT_MAX);
    upper = Vector3F(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (int i=0; i < buffers->positions.size(); ++i)
    {
        lower = VMin(Vector3F(buffers->positions[i]), lower);
        upper = VMax(Vector3F(buffers->positions[i]), upper);
    }
}

// calculates the union bounds of all the collision shapes in the scene
void OpenGLWidget::GetShapeBounds(Vector3F& totalLower, Vector3F& totalUpper)
{
    Bounds totalBounds;

    for (int i=0; i < buffers->shapeFlags.size(); ++i)
    {
        NvFlexCollisionGeometry geo = buffers->shapeGeometry[i];

        int type = buffers->shapeFlags[i]&eNvFlexShapeFlagTypeMask;

        Vector3F localLower;
        Vector3F localUpper;

        switch(type)
        {
            case eNvFlexShapeBox:
            {
                localLower = -Vector3F(geo.box.halfExtents[0], geo.box.halfExtents[1], geo.box.halfExtents[2]);
                localUpper = Vector3F(geo.box.halfExtents[0], geo.box.halfExtents[1], geo.box.halfExtents[2]);
                break;
            }
            case eNvFlexShapeSphere:
            {
                localLower = -Vector3F(geo.sphere.radius, geo.sphere.radius, geo.sphere.radius);
                localUpper = Vector3F(geo.sphere.radius, geo.sphere.radius, geo.sphere.radius);
                break;
            }
            case eNvFlexShapeCapsule:
            {
                localLower = -Vector3F(geo.capsule.halfHeight, 0.0f, 0.0f) - Vector3F(geo.capsule.radius, geo.capsule.radius, geo.capsule.radius);
                localUpper = Vector3F(geo.capsule.halfHeight, 0.0f, 0.0f) + Vector3F(geo.capsule.radius, geo.capsule.radius, geo.capsule.radius);
                break;
            }
            case eNvFlexShapeConvexMesh:
            {
                NvFlexGetConvexMeshBounds(flexLib, geo.convexMesh.mesh, &localLower[0], &localUpper[0]);

                // apply instance scaling
                localLower *= Vector3F(geo.convexMesh.scale[0], geo.convexMesh.scale[1], geo.convexMesh.scale[2]);
                localUpper *= Vector3F(geo.convexMesh.scale[0], geo.convexMesh.scale[1], geo.convexMesh.scale[2]);
                break;
            }
            case eNvFlexShapeTriangleMesh:
            {
                NvFlexGetTriangleMeshBounds(flexLib, geo.triMesh.mesh, &localLower[0], &localUpper[0]);

                // apply instance scaling
                localLower *= Vector3F(geo.triMesh.scale[0], geo.triMesh.scale[1], geo.triMesh.scale[2]);
                localUpper *= Vector3F(geo.triMesh.scale[0], geo.triMesh.scale[1], geo.triMesh.scale[2]);
                break;
            }
            case eNvFlexShapeSDF:
            {
                localLower = Vector3F();
                localUpper = Vector3F(geo.sdf.scale, geo.sdf.scale, geo.sdf.scale);
                break;
            }
        };

        // transform local bounds to world space
        Vector3F worldLower, worldUpper;
        TransformBounds(localLower, localUpper, Vector3F(buffers->shapePositions[i]), buffers->shapeRotations[i], 1.0f, worldLower, worldUpper);

        totalBounds = Union(totalBounds, Bounds(worldLower, worldUpper));
    }

    totalLower = totalBounds.lower;
    totalUpper = totalBounds.upper;
}

void OpenGLWidget::renderScene()
{
    auto DegToRad = [](float t) { return t * kPi/180.0f; };

    auto RadToDeg = [](float t) { return t * 180.0f/kPi; };

    const int numParticles = NvFlexGetActiveCount(solver);
    const int numDiffuse = buffers->diffuseCount[0];

    //---------------------------------------------------
    // use VBO buffer wrappers to allow Flex to write directly to the OpenGL buffers
    // Flex will take care of any CUDA interop mapping/unmapping during the get() operations

    if (numParticles)
    {
        fluidRenderer->UpdateFluidRenderBuffers(fluidRenderBuffers, solver, false, false);
    }

    if (numDiffuse)
    {
        fluidRenderer->UpdateDiffuseRenderBuffers(diffuseRenderBuffers, solver);
    }

    //------------------------------------
    // lighting pass

    // expand scene bounds to fit most scenes
    g_sceneLower = VMin(g_sceneLower, Vector3F(-100.0f, 0.0f, -100.0f));
    g_sceneUpper = VMax(g_sceneUpper, Vector3F(100.0f, 100.0f, 100.0f));

    Vector3F sceneExtents = g_sceneUpper - g_sceneLower;
    Vector3F sceneCenter = 0.5f*(g_sceneUpper + g_sceneLower);

    Vector3F lightDir = Vector3F(5.0f, 15.0f, 7.5f).normalized();
    Vector3F lightPos = sceneCenter + lightDir*sceneExtents.length()*lightDistance;
    Vector3F lightTarget = sceneCenter;

    // calculate tight bounds for shadow frustum
    float lightFov = 2.0f*atanf((g_sceneUpper - sceneCenter).length() / (lightPos - sceneCenter).length());

    // scale and clamp fov for aesthetics
    lightFov = clamp(lightFov, DegToRad(25.0f), DegToRad(65.0f));

    Matrix4 lightPerspective, lightView, lightTransform;
    lightPerspective.perspective(RadToDeg(lightFov), 1.0f, 1.0f, 1000.0f);
    lightView.lookAt(lightPos, lightTarget, Vector3F(0.f, 1.f, 0.f));
    lightTransform = lightPerspective*lightView;

    // radius used for drawing
    float radius = std::max(params->solidRestDistance, params->fluidRestDistance)*0.5f*trash->pointScale;

    //-------------------------------------
    // shadowing pass

    /*if (g_meshSkinIndices.size())
        SkinMesh();*/

    // create shadow maps
    shadowMap->ShadowBegin(this);

    setStupidView(lightView, lightPerspective);
    glDisable(GL_CULL_FACE);

    if (trash->drawMesh)
        DrawMesh(mesh, meshColor);

    //DrawShapes();

    int shadowParticles = numParticles - trash->numSolidParticles;
    int shadowParticlesOffset = trash->numSolidParticles;

    /*if (buffers->activeIndices.size())
        DrawPoints(fluidRenderBuffers, shadowParticles, shadowParticlesOffset, radius, 2048, 1.0f, lightFov, lightPos, lightTarget, lightTransform, *shadowMap, trash->drawDensity);*/

    shadowMap->ShadowEnd(this, msaaFbo);

    //----------------
    // lighting pass

    //BindSolidShader(lightPos, lightTarget, lightTransform, Vector4F(0.f, 0.f, 0.f, 0.005));

    float fov = kPi / 4.0f;
    float aspect = float(width()) / height();

    Matrix4 proj, view;
    proj.perspective(RadToDeg(fov), aspect, 0.01f, 1000.f);
    view.lookAt({0.f, 0.f, 2.f}, {0.f, 0.f, 0.f}, Vector3F(0.f, 1.f, 0.f));

    setStupidView(view, proj);
    glEnable(GL_CULL_FACE);

    /*DrawPlanes((Vec4*)g_params.planes, g_params.numPlanes, g_drawPlaneBias);

    if (g_drawMesh)
        DrawMesh(g_mesh, g_meshColor);


    DrawShapes();*/

    //UnbindSolidShader();

    // first pass of diffuse particles (behind fluid surface)
    fluidRenderer->RenderDiffuse(diffuseRenderBuffers,
                                 numDiffuse,
                                 radius*trash->diffuseScale,
                                 width(),
                                 aspect,
                                 fov,
                                 trash->diffuseColor,
                                 lightPos,
                                 lightTarget,
                                 lightTransform,
                                 shadowMap.data(),
                                 trash->diffuseMotionScale,
                                 trash->diffuseInscatter,
                                 trash->diffuseOutscatter,
                                 trash->diffuseShadow,
                                 false);


    // render fluid surface
    fluidRenderer->RenderFluid(fluidRenderBuffers,
                               numParticles - trash->numSolidParticles,
                               trash->numSolidParticles,
                               radius,
                               width(),
                               aspect,
                               fov,
                               lightPos,
                               lightTarget,
                               lightTransform,
                               shadowMap.data(),
                               trash->fluidColor,
                               trash->blur,
                               trash->ior,
                               msaaFbo,
                               mesh);

    fluidRenderer->RenderDiffuse(diffuseRenderBuffers,
                                 numDiffuse,
                                 radius*trash->diffuseScale,
                                 width(),
                                 aspect,
                                 fov,
                                 trash->diffuseColor,
                                 lightPos,
                                 lightTarget,
                                 lightTransform,
                                 shadowMap.data(),
                                 trash->diffuseMotionScale,
                                 trash->diffuseInscatter,
                                 trash->diffuseOutscatter,
                                 trash->diffuseShadow,
                                 true);
}

void OpenGLWidget::setView()
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera->getProjection().constData());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera->getView().constData());
}

void OpenGLWidget::setStupidView(const Matrix4& view, const Matrix4& projection)
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.constData());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.constData());
}

void OpenGLWidget::DrawMesh(const Mesh* m, Color3F& color)
{
    if (m)
    {
        glVerify(glColor3fv(&color[0]));
        glVerify(glSecondaryColor3fv(&color[0]));

        glVerify(glBindBuffer(GL_ARRAY_BUFFER, 0));
        glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        glVerify(glEnableClientState(GL_NORMAL_ARRAY));
        glVerify(glEnableClientState(GL_VERTEX_ARRAY));

        glVerify(glNormalPointer(GL_FLOAT, sizeof(float) * 3, &m->m_normals[0]));
        glVerify(glVertexPointer(3, GL_FLOAT, sizeof(float) * 3, &m->m_positions[0]));

        if (m->m_colours.size())
        {
            glVerify(glEnableClientState(GL_COLOR_ARRAY));
            glVerify(glColorPointer(4, GL_FLOAT, 0, &m->m_colours[0]));
        }

        glVerify(glDrawElements(GL_TRIANGLES, m->GetNumFaces() * 3, GL_UNSIGNED_INT, &m->m_indices[0]));

        glVerify(glDisableClientState(GL_VERTEX_ARRAY));
        glVerify(glDisableClientState(GL_NORMAL_ARRAY));

        if (m->m_colours.size())
            glVerify(glDisableClientState(GL_COLOR_ARRAY));
    }
}

// vertex shader
static const char *vertexShader = "#version 130\n" QT_STRINGIFY(

uniform mat4 lightTransform;
uniform vec3 lightDir;
uniform float bias;
uniform vec4 clipPlane;
uniform float expand;

uniform mat4 objectTransform;

void main()
{
    vec3 n = normalize((objectTransform*vec4(gl_Normal, 0.0)).xyz);
    vec3 p = (objectTransform*vec4(gl_Vertex.xyz, 1.0)).xyz;

    // calculate window-space point size
    gl_Position = gl_ModelViewProjectionMatrix * vec4(p + expand*n, 1.0);

    gl_TexCoord[0].xyz = n;
    gl_TexCoord[1] = lightTransform*vec4(p + n*bias, 1.0);
    gl_TexCoord[2] = gl_ModelViewMatrix*vec4(lightDir, 0.0);
    gl_TexCoord[3].xyz = p;
    gl_TexCoord[4] = gl_Color;
    gl_TexCoord[5] = gl_MultiTexCoord0;
    gl_TexCoord[6] = gl_SecondaryColor;
    gl_TexCoord[7] = gl_ModelViewMatrix*vec4(gl_Vertex.xyz, 1.0);

    gl_ClipDistance[0] = dot(clipPlane,vec4(gl_Vertex.xyz, 1.0));
}
);

// pixel shader for rendering points as shaded spheres
const char *fragmentShader = "" QT_STRINGIFY(

uniform vec3 lightDir;
uniform vec3 lightPos;
uniform float spotMin;
uniform float spotMax;
uniform vec3 color;
uniform vec4 fogColor;

uniform sampler2DShadow shadowTex;
uniform vec2 shadowTaps[12];

uniform sampler2D tex;
uniform bool sky;

uniform bool grid;
uniform bool texture;

float sqr(float x) { return x*x; }

// sample shadow map
float shadowSample()
{
    vec3 pos = vec3(gl_TexCoord[1].xyz/gl_TexCoord[1].w);
    vec3 uvw = (pos.xyz*0.5)+vec3(0.5);

    // user clip
    if (uvw.x  < 0.0 || uvw.x > 1.0)
        return 1.0;
    if (uvw.y < 0.0 || uvw.y > 1.0)
        return 1.0;

    float s = 0.0;
    float radius = 0.002;

    const int numTaps = 12;

    for (int i=0; i < numTaps; i++)
    {
        s += shadow2D(shadowTex, vec3(uvw.xy + shadowTaps[i]*radius, uvw.z)).r;
    }

    s /= numTaps;
    return s;
}

float filterwidth(vec2 v)
{
  vec2 fw = max(abs(dFdx(v)), abs(dFdy(v)));
  return max(fw.x, fw.y);
}

vec2 bump(vec2 x)
{
    return (floor((x)/2) + 2.f * max(((x)/2) - floor((x)/2) - .5f, 0.f));
}

float checker(vec2 uv)
{
  float width = filterwidth(uv);
  vec2 p0 = uv - 0.5 * width;
  vec2 p1 = uv + 0.5 * width;

  vec2 i = (bump(p1) - bump(p0)) / width;
  return i.x * i.y + (1 - i.x) * (1 - i.y);
}

void main()
{
    // calculate lighting
    float shadow = max(shadowSample(), 0.5);

    vec3 lVec = normalize(gl_TexCoord[3].xyz-(lightPos));
    vec3 lPos = vec3(gl_TexCoord[1].xyz/gl_TexCoord[1].w);
    float attenuation = max(smoothstep(spotMax, spotMin, dot(lPos.xy, lPos.xy)), 0.05);

    vec3 n = gl_TexCoord[0].xyz;
    vec3 color = gl_TexCoord[4].xyz;

    if (!gl_FrontFacing)
    {
        color = gl_TexCoord[6].xyz;
        n *= -1.0f;
    }

    if (grid && (n.y >0.995))
    {
        color *= 1.0 - 0.25 * checker(vec2(gl_TexCoord[3].x, gl_TexCoord[3].z));
    }
    else if (grid && abs(n.z) > 0.995)
    {
        color *= 1.0 - 0.25 * checker(vec2(gl_TexCoord[3].y, gl_TexCoord[3].x));
    }

    if (texture)
    {
        color = texture2D(tex, gl_TexCoord[5].xy).xyz;
    }

    // direct light term
    float wrap = 0.0;
    vec3 diffuse = color*vec3(1.0, 1.0, 1.0)*max(0.0, (-dot(lightDir, n)+wrap)/(1.0+wrap)*shadow)*attenuation;

    // wrap ambient term aligned with light dir
    vec3 light = vec3(0.03, 0.025, 0.025)*1.5;
    vec3 dark = vec3(0.025, 0.025, 0.03);
    vec3 ambient = 4.0*color*mix(dark, light, -dot(lightDir, n)*0.5 + 0.5)*attenuation;

    vec3 fog = mix(vec3(fogColor), diffuse + ambient, exp(gl_TexCoord[7].z*fogColor.w));

    gl_FragColor = vec4(pow(fog, vec3(1.0/2.2)), 1.0);
}
);

void OpenGLWidget::BindSolidShader(Vector3F& lightPos, const Vector3F& lightTarget, const Matrix4& lightTransform, Vector4F& fogColor)
{
    glVerify(glViewport(0, 0, width(), height()));

    if (!diffuseProgram)
        diffuseProgram = FluidRenderer::CompileProgram(this, vertexShader, fragmentShader);

    if (diffuseProgram)
    {
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        glVerify(glUseProgram(diffuseProgram));
        glVerify(glUniform1i(glGetUniformLocation(diffuseProgram, "grid"), 0));
        glVerify(glUniform1f( glGetUniformLocation(diffuseProgram, "spotMin"), 0.5f));
        glVerify(glUniform1f( glGetUniformLocation(diffuseProgram, "spotMax"), 1.f));
        glVerify(glUniform4fv( glGetUniformLocation(diffuseProgram, "fogColor"), 1, &fogColor[0]));

        static Matrix4 indentity;
        glVerify(glUniformMatrix4fv( glGetUniformLocation(diffuseProgram, "objectTransform"), 1, false, indentity.constData()));

        // set shadow parameters
        shadowMap->ShadowApply(this, diffuseProgram, lightPos, lightTarget, lightTransform);
    }
}
