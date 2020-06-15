#include "gtmeshpoints.h"

#include <QOpenGLBuffer>

GtMeshPoints::GtMeshPoints()
{}

void GtMeshPoints::SetPoints(const QVector<Point3F>& points)
{
    m_points.clear();

    for(const auto& point : points) {
        m_points.append({ point, Color3F(0.f, 1.f, 1.f) });
    }
}

void GtMeshPoints::SetPoints(const QVector<ColoredVertex3F>& points)
{
    m_points.clear();

    for(const auto& point : points) {
        m_points.append(point);
    }
}

bool GtMeshPoints::buildMesh()
{
    m_verticesCount = m_points.size();

    m_vbo->bind();
    m_vbo->allocate(m_points.data(), m_verticesCount * sizeof(ColoredVertex3F));
    m_vbo->release();

    m_points.clear();

    return true;
}

void GtMeshPoints::bindVAO(OpenGLFunctions* f)
{
    m_vbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,3,GL_FLOAT,false,sizeof(ColoredVertex3F),nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,3,GL_FLOAT,false,sizeof(ColoredVertex3F),(const void*)sizeof(Point3F));
}
