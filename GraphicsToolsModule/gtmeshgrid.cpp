#include "gtmeshgrid.h"

#include <QOpenGLBuffer>

GtMeshGrid::GtMeshGrid(qint32 width, qint32 height, qint32 sections)
    : m_width(width)
    , m_height(height)
    , m_sections(sections)

{

}

bool GtMeshGrid::buildMesh()
{
    /*    <--w-->
     *   /_______\
     *  /|_s_|___|
     * h |___|___|
     *  \|___|___|
     *  s - count of sections at the line. The same count at the column, total sections count is s * s
     *  w - real width
     *  h - real height
    */
    /**/
    qint32 sectionsPlusOne = m_sections + 1;
    float hStep = float(m_height) / m_sections;
    float wStep = float(m_width) / m_sections;

    m_verticesCount = m_sections * (2 * m_sections + 2) + (2 * m_sections - 2);
    ColoredVertex2F* vertices = new ColoredVertex2F[m_verticesCount];

    bool white = false;
    auto getColor = [&white]() {
        return white ? Color3F(1.f, 1.f, 1.f) : Color3F(0.f, 0.f, 0.f);
    };
    qint32 currentIndex = 0;

    for(qint32 j(1); j < sectionsPlusOne; j++) {
        for(qint32 i(0); i < sectionsPlusOne; i++) {
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * i, hStep * j), getColor() };
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * i, hStep * (j - 1)), getColor() };
            white = !white;
        }
        if(j != m_sections) {
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * m_sections, hStep * (j - 1)), getColor() };
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(0.f, hStep * (j + 1)), getColor() };
        }
    }

    m_vbo->bind();
    m_vbo->allocate(vertices, m_verticesCount * sizeof(ColoredVertex2F));
    m_vbo->release();

    delete [] vertices;

    /**/
    /*
    qint32 sectionsPlusOne = m_sections + 1;
    float hStep = float(m_height) / m_sections;
    float wStep = float(m_width) / m_sections;

    m_verticesCount = m_sections * m_sections * 6;
    ColoredVertex2F* vertices = new ColoredVertex2F[m_verticesCount];

    bool white = false;
    auto getColor = [&white]() {
        return white ? Color3F{ 1.f, 1.f, 1.f } : Color3F{ 0.5f, 0.5f, 0.5f };
    };
    qint32 currentIndex = 0;

    for(qint32 j(1); j < sectionsPlusOne; j++) {
        for(qint32 i(1); i < sectionsPlusOne; i++) {
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * i, hStep * j), getColor() };
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * i, hStep * (j - 1)), getColor() };
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * (i - 1), hStep * (j - 1)), getColor() };

            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * i, hStep * j), getColor() };
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * (i - 1), hStep * (j - 1)), getColor() };
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * (i - 1), hStep * j), getColor() };

            white = !white;
        }
    }

    m_vbo->bind();
    m_vbo->allocate(vertices, m_verticesCount * sizeof(ColoredVertex2F));
    m_vbo->release();

    delete [] vertices;
    /**/
    return true;
}

void GtMeshGrid::bindVAO(OpenGLFunctions* f)
{
    m_vbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2,GL_FLOAT,false,sizeof(ColoredVertex2F),nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,3,GL_FLOAT,false,sizeof(ColoredVertex2F),(const void*)sizeof(Point2F));
}
