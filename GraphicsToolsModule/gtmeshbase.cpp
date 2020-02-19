#include "gtmeshbase.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

GtMeshBase::GtMeshBase()
    : m_verticesCount(0)
    , m_visible(true)
{

}

GtMeshBase::~GtMeshBase()
{
}

void GtMeshBase::Update()
{
    m_visible = buildMesh();
}

void GtMeshBase::Initialize(OpenGLFunctions* f)
{
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vao = new QOpenGLVertexArrayObject();

    if(m_vbo->create() && m_vao->create()) {
        m_visible = buildMesh();
        QOpenGLVertexArrayObject::Binder binder(m_vao.data());
        bindVAO(f);
    }
    else {
        m_visible = false;
    }
}

void GtMeshBase::Draw(gRenderType renderType, OpenGLFunctions* f)
{
    QOpenGLVertexArrayObject::Binder binder(m_vao.data());
    f->glDrawArrays(renderType, 0, m_verticesCount);
}

GtMeshIndicesBase::GtMeshIndicesBase(gIndicesType itype)
    : m_indicesCount(0)
    , m_indicesType(itype)
{

}

GtMeshIndicesBase::~GtMeshIndicesBase()
{
}

void GtMeshIndicesBase::Initialize(OpenGLFunctions* f)
{
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vboIndices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_vao = new QOpenGLVertexArrayObject();

    if(m_vbo->create() && m_vboIndices->create() && m_vao->create()) {
        m_visible = buildMesh();
        QOpenGLVertexArrayObject::Binder binder(m_vao.data());
        bindVAO(f);
    }
    else {
        m_visible = false;
    }
}

void GtMeshIndicesBase::Draw(gRenderType renderType, OpenGLFunctions* f)
{
    m_vao->bind();
    m_vboIndices->bind();
    f->glDrawElements(renderType, m_indicesCount, m_indicesType, (const void*)0);
    m_vao->release();
}
