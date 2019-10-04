#include "gttexture2D.h"

#include <QImage>
#include <QOpenGLContext>
#include "DDS/nv_dds.h"

GtTexture::~GtTexture()
{
    if(m_textureId) f->glDeleteTextures(1, &m_textureId);
}

void GtTexture::SetSize(quint32 w, quint32 h)
{
    QSize new_size = QSize(w,h);
    if(m_size != new_size) {
        m_size = new_size;
        m_allocated = false;
    }
}

void GtTexture::SetInternalFormat(gTexInternalFormat internal_format)
{
    if(this->m_internalFormat != internal_format)
    {
        this->m_internalFormat = internal_format;
        m_allocated = false;
    }
}

void GtTexture::Bind()
{
    f->glBindTexture(m_target, m_textureId);
}

void GtTexture::Bind(quint32 unit)
{
    f->glActiveTexture(unit);
    f->glBindTexture(m_target, m_textureId);
}

void GtTexture::Release()
{
    f->glBindTexture(GetTarget(), m_textureId);
}

bool GtTexture::Create()
{
//    Q_ASSERT(QOpenGLContext::currentContext() && (OpenGLFunctions*)QOpenGLContext::currentContext()->functions() == f);
    if(IsCreated())
        return true;
    f->glGenTextures(1, &m_textureId);
    return IsCreated();
}

bool GtTexture::IsValid() const
{
    return m_textureId && !m_size.isNull();
}

GtTexture* GtTexture::Create(OpenGLFunctions* f, gTexTarget target, gTexInternalFormat internal_format, const SizeI& size, const GtTextureFormat* format)
{
    GtTexture* result = nullptr;
    switch (target) {
    case GL_TEXTURE_2D:
        result = new GtTexture2D(f);
        break;
    default:
        break;
    }
    Q_ASSERT(result);
    result->SetInternalFormat(internal_format);
    result->SetSize(size.width(), size.height());
    result->Allocate(*format);
    return result;
}

GtTexture2D::GtTexture2D(OpenGLFunctions* f)
    : GtTexture(f, GL_TEXTURE_2D)
{

}

void GtTexture2D::LoadImg(const QString& img_file)
{
    if(!Create()) {
        qCWarning(LC_SYSTEM) << "Unable to create texture";
        return;
    }

    QImage img(img_file);
    if(img.isNull()) {
        qCWarning(LC_SYSTEM) << "Cannot read image" << img_file;
        return;
    }

    QImage gl_img = img.convertToFormat(QImage::Format_RGBA8888);
    SetSize(img.width(), img.height());
    SetInternalFormat(GL_RGBA);
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GtTextureFormat format;
    format.Pixels = gl_img.constBits();
    format.PixelFormat = GL_RGBA;
    format.PixelType = GL_UNSIGNED_BYTE;
    format.WrapS = GL_REPEAT;
    format.WrapT = GL_REPEAT;
    Allocate(format);
}

void GtTexture2D::Load(const QString& dds_file)
{
    nv_dds::DDSImage img(f);
    img.Load(dds_file.toStdString());
    if(Create()) {
        Bind();
        m_size.setWidth(img.GetWidth());
        m_size.setHeight(img.GetHeight());
        m_internalFormat = img.GetComponents();
        img.UploadTexture2D();
        m_allocated = true;
        Release();
    }
}

void GtTexture2D::bindTexture(OpenGLFunctions* f, gTexUnit unit, gTexID id)
{
    f->glActiveTexture(unit + GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, id);
}

void GtTexture2D::Allocate(const GtTextureFormat& format)
{
    if(IsCreated() || Create()) {
        GtTextureBinder binder(this);
        if(!m_allocated) {
            f->glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_size.width(), m_size.height(), 0, format.PixelFormat, format.PixelType, format.Pixels);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, format.MinFilter);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, format.MagFilter);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format.WrapS);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format.WrapT);
            m_allocated = true;
        }
        else {
            f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.width(), m_size.height(), format.PixelFormat, format.PixelType, format.Pixels);
        }
    }
}


GtTexture2DMultisampled::GtTexture2DMultisampled(OpenGLFunctions* f, quint32 samples)
    : GtTexture(f, GL_TEXTURE_2D_MULTISAMPLE)
    , m_samples(samples)
{
    Q_ASSERT(samples > 1 && (samples % 2) == 0);
}

void GtTexture2DMultisampled::Allocate(const GtTextureFormat&)
{
    if(IsCreated() || Create()) {
        GtTextureBinder binder(this);
        f->glTexImage2DMultisample(GL_TEXTURE_2D, m_samples, m_internalFormat, m_size.width(), m_size.height(), true);
    }
}
