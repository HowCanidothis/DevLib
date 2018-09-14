#include "gtmeshgrid.h"

#include <QOpenGLBuffer>

GtMeshGrid::GtMeshGrid(qint32 width, qint32 height, qint32 sections)
    : GtMeshBase(GL_TRIANGLES)
    , _width(width)
    , _height(height)
    , _sections(sections)

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
    /*
    qint32 sectionsPlusOne = _sections + 1;
    float hStep = float(_height) / _sections;
    float wStep = float(_width) / _sections;

    vertices_count = _sections * (2 * _sections + 2) + (2 * _sections - 2);
    ColoredVertex2F* vertices = new ColoredVertex2F[vertices_count];

    bool white = false;
    auto getColor = [&white]() {
        return white ? Quantity_Color(1.f, 1.f, 1.f, Quantity_TOC_RGB) : Quantity_Color(0.f, 0.f, 0.f, Quantity_TOC_RGB);
    };
    qint32 currentIndex = 0;

    for(qint32 j(1); j < sectionsPlusOne; j++) {
        for(qint32 i(0); i < sectionsPlusOne; i++) {
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * i, hStep * j), getColor() };
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * i, hStep * (j - 1)), getColor() };
            white = !white;
        }
        if(j != _sections) {
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(wStep * _sections, hStep * (j - 1)), getColor() };
            vertices[currentIndex++] = ColoredVertex2F{ Point2F(0.f, hStep * (j + 1)), getColor() };
        }
    }

    vbo->bind();
    vbo->allocate(vertices, vertices_count * sizeof(ColoredVertex2F));
    vbo->release();

    delete [] vertices;

    /**/
    /**/
    qint32 sectionsPlusOne = _sections + 1;
    float hStep = float(_height) / _sections;
    float wStep = float(_width) / _sections;

    vertices_count = _sections * _sections * 6;
    ColoredVertex2F* vertices = new ColoredVertex2F[vertices_count];

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

    vbo->bind();
    vbo->allocate(vertices, vertices_count * sizeof(ColoredVertex2F));
    vbo->release();

    delete [] vertices;
    /**/
    return true;
}

void GtMeshGrid::bindVAO(OpenGLFunctions* f)
{
    vbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2,GL_FLOAT,false,sizeof(ColoredVertex2F),nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1,3,GL_FLOAT,false,sizeof(ColoredVertex2F),(const void*)sizeof(Point2F));
}
