#include "gtmeshstatedpoints.h"

#include <QOpenGLBuffer>

GtMeshStatedPoints::GtMeshStatedPoints()
{

}

void GtMeshStatedPoints::SetPoints(const QVector<StatedVertex3F>& points)
{
    m_points.clear();

    for(const auto& point : points) {
        m_points.append(point);
    }
}

bool GtMeshStatedPoints::buildMesh()
{
    m_verticesCount = m_points.size();

    m_vbo->bind();
    m_vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vbo->allocate(m_points.data(), m_verticesCount * sizeof(StatedVertex3F));
    m_vbo->release();

    m_points.clear();

    return true;
}

void GtMeshStatedPoints::bindVAO(OpenGLFunctions* f)
{
    m_vbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,3,GL_FLOAT,false,sizeof(StatedVertex3F),nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribIPointer(1,1,GL_UNSIGNED_BYTE,sizeof(StatedVertex3F),(const void*)sizeof(Point3F));
}
