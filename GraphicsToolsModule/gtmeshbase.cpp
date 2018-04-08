#include "gtmeshbase.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

GtMeshBase::GtMeshBase(gRenderType type)
    : vbo(nullptr)
    , vao(nullptr)
    , vertices_count(0)
    , render_type(type)
    , visible(true)
{

}

GtMeshBase::~GtMeshBase()
{
    delete vbo;
    delete vao;
}

void GtMeshBase::update()
{
    visible = buildMesh();
}

void GtMeshBase::initialize(OpenGLFunctions* f)
{
    vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vao = new QOpenGLVertexArrayObject();

    if(vbo->create() && vao->create()) {
        visible = buildMesh();
        QOpenGLVertexArrayObject::Binder binder(vao);
        bindVAO(f);
    }
    else {
        visible = false;
    }
}

void GtMeshBase::draw(OpenGLFunctions* f)
{
    QOpenGLVertexArrayObject::Binder binder(vao);
    f->glDrawArrays(render_type, 0, vertices_count);
}

GtMeshIndicesBase::GtMeshIndicesBase(gRenderType type, gIndicesType itype)
    : GtMeshBase(type)
    , vbo_indices(nullptr)
    , indices_count(0)
    , indices_type(itype)
{

}

GtMeshIndicesBase::~GtMeshIndicesBase()
{
    delete vbo_indices;
}

void GtMeshIndicesBase::initialize(OpenGLFunctions* f)
{
    vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo_indices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    vao = new QOpenGLVertexArrayObject();

    if(vbo->create() && vbo_indices->create() && vao->create()) {
        visible = buildMesh();
        QOpenGLVertexArrayObject::Binder binder(vao);
        bindVAO(f);
    }
    else {
        visible = false;
    }
}

void GtMeshIndicesBase::draw(OpenGLFunctions* f)
{
    vao->bind();
    vbo_indices->bind();
    f->glDrawElements(render_type, indices_count, indices_type, (const void*)0);
    vao->release();
}
