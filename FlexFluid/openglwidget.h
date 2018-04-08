#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include "GraphicsTools/gt_decl.h"

struct Emitter
{
    Emitter() : mSpeed(0.0f), mEnabled(false), mLeftOver(0.0f), mWidth(8)   {}

    Vector3F mPos;
    Vector3F mDir;
    Vector3F mRight;
    float mSpeed;
    bool mEnabled;
    float mLeftOver;
    int mWidth;
};

struct Rope
{
    std::vector<int> mIndices;
};

struct Mesh;

class OpenGLWidget : public QOpenGLWidget, protected OpenGLFunctions
{
    Q_OBJECT
    ScopedPointer<class GtCamera> camera;
    ScopedPointer<class QOpenGLShaderProgram> program;
    ScopedPointer<class QOpenGLBuffer> vbo;
    ScopedPointer<class QOpenGLBuffer> vbo_indices;
    ScopedPointer<class QOpenGLVertexArrayObject> vao;

    ScopedPointer<struct FluidRenderer> fluidRenderer;
    struct FluidRenderBuffers* fluidRenderBuffers;
    struct DiffuseRenderBuffers* diffuseRenderBuffers;

    QPoint resolutional(const QPoint& p) const;
    Vector3F last_plane_position;
    QPoint last_screen_position;
    quint32 indices;

    GLuint msaaFbo;
    GLuint msaaColorBuf;
    GLuint msaaDepthBuf;

    // mesh used for deformable object rendering
    Mesh* mesh;
    std::vector<int> meshSkinIndices;
    std::vector<float> meshSkinWeights;
    std::vector<Point3F> meshRestPositions;
    Vector3F meshColor = Vector3F(0.9f, 0.9f, 0.9f);
    const int numSkinWeights = 4;

    std::vector<Emitter> emitters { Emitter() };
    std::vector<Rope> ropes;

    ScopedPointer<struct TrashConstants> trash;
    ScopedPointer<struct SimBuffers> buffers;
    ScopedPointer<struct ShadowMap> shadowMap;
    ScopedPointer<struct NvFlexParams> params;
    struct NvFlexLibrary* flexLib;
    struct NvFlexSolver* solver;
    qint32 flex_device;

    Vector3F g_sceneLower;
    Vector3F g_sceneUpper;

    float lightDistance = 1.f;
    OpenGLFunctions* f;

    GLuint diffuseProgram = 0;
public:
    OpenGLWidget(QWidget *parent = 0);
    ~OpenGLWidget();

    // QOpenGLWidget interface
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

    void reshapeRender(qint32 w, qint32 h);

    void initScene();

    void initFluidBlockScene();

    void AddRandomConvex(int numPlanes, const Vector3F& position, float minDist, float maxDist, const Vector3F& axis, float angle);
    void CreateParticleShape(const Mesh* srcMesh,
                             const Vector3F& lower,
                             const Vector3F& scale,
                             float rotation,
                             float spacing,
                             const Vector3F& velocity,
                             float invMass,
                             bool rigid,
                             float rigidStiffness,
                             int phase,
                             bool skin,
                             float jitter=0.005f,
                             const Vector3F& skinOffset=Vector3F(),
                             float skinExpand=0.0f,
                             const Vector4F& color=Vector4F(),
                             float springStiffness=0.0f);
    void CreateParticleGrid(const Vector3F& lower, int dimx, int dimy, int dimz, float radius, const Vector3F& velocity, float invMass, bool rigid, float rigidStiffness, int phase, float jitter=0.005f);
    void CreateSpring(int i, int j, float stiffness, float give=0.0f);

    void GetParticleBounds(Vector3F& lower, Vector3F& upper);
    void GetShapeBounds(Vector3F& totalLower, Vector3F& totalUpper);

    void renderScene();

    void setView();
    void setStupidView(const Matrix4& view, const Matrix4& projection);

    void DrawMesh(const Mesh* m, Color3F& color);

    void BindSolidShader(Vector3F& lightPos, const Vector3F& lightTarget, const Matrix4& lightTransform, Vector4F& fogColor);

};

#endif // OPENGLWIDGET_H
