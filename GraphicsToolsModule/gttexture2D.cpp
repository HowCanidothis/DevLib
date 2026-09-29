#include "gttexture2D.h"

#include <QDir>
#include <QFileInfoList>
#include <QImage>
#include <QOpenGLContext>
#include "DDS/nv_dds.h"

GtTexture::~GtTexture()
{
    if(m_textureId) f->glDeleteTextures(1, &m_textureId);
}

void GtTexture::SetData(const void* pixels)
{
    m_format.Pixels = pixels;
    Allocate();
}

void GtTexture::SetFormat(const GtTextureFormat& format)
{
    m_format = format;
    m_allocated = false;
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
    if(m_internalFormat != internal_format)
    {
        m_internalFormat = internal_format;
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
    result->SetFormat(*format);
    result->Allocate();
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
    m_format.Pixels = gl_img.constBits();
    m_format.PixelFormat = GL_RGBA;
    m_format.PixelType = GL_UNSIGNED_BYTE;
    Allocate();
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

void GtTexture2D::Allocate()
{
    if(IsCreated() || Create()) {
        GtTextureBinder binder(this);
        if(!m_allocated) {
            f->glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_size.width(), m_size.height(), 0, m_format.PixelFormat, m_format.PixelType, m_format.Pixels);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_format.MinFilter);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_format.MagFilter);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_format.WrapS);
            f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_format.WrapT);
            if(m_format.MipMapLevels != 0) {
                f->glGenerateMipmap(GL_TEXTURE_2D);
            }
            m_allocated = true;
        } else {
            f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.width(), m_size.height(), m_format.PixelFormat, m_format.PixelType, m_format.Pixels);
        }
    }
}


GtTexture2DMultisampled::GtTexture2DMultisampled(OpenGLFunctions* f, quint32 samples)
    : GtTexture(f, GL_TEXTURE_2D_MULTISAMPLE)
    , m_samples(samples)
{
    Q_ASSERT(samples > 1 && (samples % 2) == 0);
}

void GtTexture2DMultisampled::Allocate()
{
    if(IsCreated() || Create()) {
        GtTextureBinder binder(this);
        f->glTexStorage2DMultisample(m_target, m_samples, m_internalFormat, m_size.width(), m_size.height(), GL_TRUE);
    }
}

GtTexture3D::GtTexture3D(OpenGLFunctions* f, gTexTarget target)
    : GtTexture(f, target)
    , m_depth(0)
{
}

void GtTexture3D::SetDepth(quint32 depth)
{
    if (m_depth != depth) {
        m_depth = depth;
        m_allocated = false;
    }
}

void GtTexture3D::LoadImages(const QString& path)
{
    // 1. Scan the directory and filter for target image formations dynamically
    QDir directory(path);
    if (!directory.exists()) {
        qCWarning(LC_SYSTEM) << "GtTexture3D: Specified formations folder path does not exist:" << path;
        return;
    }

    QStringList nameFilters;
    nameFilters << "*.png";

    QFileInfoList fileList = directory.entryInfoList(
        nameFilters,
        QDir::Files,
        QDir::Name  // Alphabetical sorting handles Basalt -> Coal -> Dolomite in order
    );

    if (fileList.isEmpty()) {
        qCWarning(LC_SYSTEM) << "GtTexture3D: No .png files discovered within path folder:" << path;
        return;
    }

    if (!IsCreated() && !Create()) {
        qCWarning(LC_SYSTEM) << "GtTexture3D: Unable to create texture handle";
        return;
    }

    // Inspect the first baseline slice to auto-extract dimensions uniformly
    QString firstFilePath = fileList.first().absoluteFilePath();
    QImage firstImg(firstFilePath);
    if (firstImg.isNull()) {
        qCWarning(LC_SYSTEM) << "GtTexture3D: Cannot read first layout file:" << firstFilePath;
        return;
    }

    // Configure structural dimensions derived dynamically from data discovery
    SetSize(firstImg.width(), firstImg.height());
    SetDepth(fileList.size()); // Adjusts array count on the fly

    // Allocate the mutable storage structure via your framework's abstract layer pipeline
    // This calls your Allocate() method, using your configured instance m_format filters/wrapping safely
    Allocate();

    // Sequentially stream individual images straight into designated layers (Z depth offsets)
    Bind();
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (int i = 0; i < fileList.size(); ++i) {
        QString currentFilePath = fileList.at(i).absoluteFilePath();
        QImage img(currentFilePath);
        if (img.isNull() || img.size() != firstImg.size()) {
            qCWarning(LC_SYSTEM) << "GtTexture3D: Failed to stream sub-image layer at array index:" << i << "-" << currentFilePath;
            continue;
        }

        QImage gl_img = img.convertToFormat(QImage::Format_RGBA8888);

        f->glTexSubImage3D(
            m_target,
            0,                          // Mipmap level
            0, 0, i,                    // xOffset, yOffset, zOffset (zOffset is our array slot layer!)
            m_size.width(),             // Layer width
            m_size.height(),            // Layer height
            1,                          // Updating exactly 1 array slice depth width
            m_format.PixelFormat,       // Uses the runtime parameter instance formats safely
            m_format.PixelType,         // Uses the runtime parameter instance types safely
            gl_img.constBits()
        );
    }

    // Safely generate mipmaps down the line if requested by format presets
    if (m_format.MipMapLevels != 0) {
        f->glGenerateMipmap(m_target);
    }

    Release();
}

void GtTexture3D::Allocate()
{
    if (IsCreated() || Create()) {
        GtTextureBinder binder(this);
        if (!m_allocated) {
            // Allocate blank device context container to span W x H x Layers
            f->glTexImage3D(
                m_target,
                0,
                m_internalFormat,
                m_size.width(),
                m_size.height(),
                m_depth,
                0,
                m_format.PixelFormat,
                m_format.PixelType,
                nullptr // Kept nullptr: sub-allocated systematically in LoadImages pass
            );

            // Apply texture sampler constraints uniform with your framework filters
            f->glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_format.MinFilter);
            f->glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_format.MagFilter);
            f->glTexParameteri(m_target, GL_TEXTURE_WRAP_S, m_format.WrapS);
            f->glTexParameteri(m_target, GL_TEXTURE_WRAP_T, m_format.WrapT);

            // Texture Arrays use an R coordinate component for layer wrapping rules
            f->glTexParameteri(m_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            m_allocated = true;
        }
    }
}
