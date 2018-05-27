#include "gttexture2D.h"

#include <QImage>
#include <QOpenGLContext>
#include "DDS/nv_dds.h"

GtTexture::~GtTexture()
{
    if(texture_id) f->glDeleteTextures(1, &texture_id);
}

void GtTexture::setSize(quint32 w, quint32 h)
{
    QSize new_size = QSize(w,h);
    if(size != new_size) {
        size = new_size;
        allocated = false;
    }
}

void GtTexture::setInternalFormat(gTexInternalFormat internal_format)
{
    if(this->internal_format != internal_format)
    {
        this->internal_format = internal_format;
        allocated = false;
    }
}

void GtTexture::bind()
{
    f->glBindTexture(target, texture_id);
}

void GtTexture::bind(quint32 unit)
{
    f->glActiveTexture(unit);
    f->glBindTexture(target, texture_id);
}

void GtTexture::release()
{
    f->glBindTexture(getTarget(), texture_id);
}

bool GtTexture::create()
{
//    Q_ASSERT(QOpenGLContext::currentContext() && (OpenGLFunctions*)QOpenGLContext::currentContext()->functions() == f);
    if(isCreated())
        return true;
    f->glGenTextures(1, &texture_id);
    return isCreated();
}

bool GtTexture::isValid() const
{
    return texture_id && !size.isNull();
}

GtTexture* GtTexture::create(OpenGLFunctions* f, gTexTarget target, gTexInternalFormat internal_format, const SizeI& size, const GtTextureFormat* format)
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
    result->setInternalFormat(internal_format);
    result->setSize(size.width(), size.height());
    result->allocate(*format);
    return result;
}

GtTexture2D::GtTexture2D(OpenGLFunctions* f)
    : GtTexture(f, GL_TEXTURE_2D)
{

}

void GtTexture2D::loadImage(const QString& img_file)
{
    LOGOUT;
    if(!create()) {
        log.Warning() << "Unable to create texture";
        return;
    }

    QImage img(img_file);
    if(img.isNull()) {
        log.Warning() << "Cannot read image" << img_file;
        return;
    }

    QImage gl_img = img.convertToFormat(QImage::Format_RGBA8888);
    setSize(img.width(), img.height());
    setInternalFormat(GL_RGBA);
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GtTextureFormat format;
    format.pixels = gl_img.constBits();
    format.pixels_format = GL_RGBA;
    format.pixels_type = GL_UNSIGNED_BYTE;
    format.wrap_s = GL_REPEAT;
    format.wrap_t = GL_REPEAT;
    allocate(format);
}

void GtTexture2D::load(const QString& dds_file)
{
    nv_dds::CDDSImage img(f);
    img.load(dds_file.toStdString());
    if(create()) {
        bind();
        size.setWidth(img.get_width());
        size.setHeight(img.get_height());
        internal_format = img.get_components();
        img.upload_texture2D();
        allocated = true;
        release();
    }
}

void GtTexture2D::bindTexture(OpenGLFunctions* f, gTexUnit unit, gTexID id)
{
    f->glActiveTexture(unit + GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, id);
}

void GtTexture2D::allocate(const GtTextureFormat& format)
{
    if(isCreated() || create()) {
        GtTextureBinder binder(this);
        if(!allocated) {
            f->glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size.width(), size.height(), 0, format.pixels_format, format.pixels_type, format.pixels);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, format.min_filter);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, format.mag_filter);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format.wrap_s);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format.wrap_t);
            allocated = true;
        }
        else {
            f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width(), size.height(), format.pixels_format, format.pixels_type, format.pixels);
        }
    }
}


GtTexture2DMultisampled::GtTexture2DMultisampled(OpenGLFunctions* f, quint32 samples)
    : GtTexture(f, GL_TEXTURE_2D_MULTISAMPLE)
    , samples(samples)
{
    Q_ASSERT(samples > 1 && (samples % 2) == 0);
}

void GtTexture2DMultisampled::allocate(const GtTextureFormat&)
{
    if(isCreated() || create()) {
        GtTextureBinder binder(this);
        f->glTexImage2DMultisample(GL_TEXTURE_2D, samples, internal_format, size.width(), size.height(), true);
    }
}
