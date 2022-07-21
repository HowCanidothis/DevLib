#include "gtellipselinedrawable.h"

#include <qmath.h>

#include "Objects/gtmaterial.h"
#include "Objects/gtmaterialparametermatrix.h"
#include "gtmeshbase.h"

GtEllipseLineDrawable::GtEllipseLineDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram)
    : Super(renderer)
    , AutoUpdateTransform(true)
    , Color(QColor(Qt::white))
    , Width(1.f)
    , Visible(true)
    , m_buffer(::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Point3F, QOpenGLBuffer::StaticDraw))
    , m_material(::make_scoped<GtMaterial>(GL_LINE_LOOP, shaderProgram))
    , m_recompute(0, CreateThreadNoCheckHandler())
{
    m_material->AddMesh(::make_shared<GtMesh>(m_buffer));
    m_material->AddParameter(::make_shared<GtMaterialParameterMatrix>("MVP", "mvp"));
    m_material->AddParameter(::make_shared<GtMaterialParameterBase>("COLOR", &Color.Native()));
    m_material->AddParameter(::make_shared<GtMaterialParameterBase>("MODEL_MATRIX", &Transform.Native()));


    auto recomputePoints = [this]{
        m_recompute.Call(CONNECTION_DEBUG_LOCATION, [this]{
            QVector<Point3F> points;
            float rw = Radius.Native().width();
            float rh = Radius.Native().height();

            float pi2 = M_PI * 2.f;
            float step = M_PI / 32;
            for(float i(0.f); i < pi2; i += step) {
                points.append(Vector3F(rw * cos(i), rh * sin(i), 0.f));
            }

            if(AutoUpdateTransform) {
                auto up = Up.Native();
                auto normal = Normal.Native();
                auto left = Vector3F::crossProduct(Normal, Up).normalized();

                QMatrix4x4 transform;
                transform.setColumn(0, left);
                transform.setColumn(1, up);
                transform.setColumn(2, normal);
                transform.setColumn(3, Vector4F(Position, 1.0));
                Transform = transform;
            }

            m_buffer->UpdateVertexArray(points);
        });
    };

    Width.SetSetterHandler(CreateThreadHandler());
    Radius.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, recomputePoints);
    Up.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, recomputePoints);
    Normal.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, recomputePoints);
    Position.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, recomputePoints);
}

void GtEllipseLineDrawable::drawDepth(OpenGLFunctions*)
{
}

void GtEllipseLineDrawable::draw(OpenGLFunctions* f)
{
    if(!Visible) {
        return;
    }

    f->glLineWidth(Width);
    m_material->Draw(f);
    f->glLineWidth(1.f);
}

void GtEllipseLineDrawable::onInitialize(OpenGLFunctions* f)
{
    m_buffer->Initialize(f);
}

void GtEllipseLineDrawable::onDestroy(OpenGLFunctions*)
{
}
