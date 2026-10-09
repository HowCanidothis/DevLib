#include "gtframebufferobject.h"

GtFramebufferObjectBase::~GtFramebufferObjectBase()
{
    f->glDeleteRenderbuffers(1, &m_depthRenderBuffer);
    f->glDeleteFramebuffers(1, &m_id);
}

void GtFramebufferObjectBase::BindRead()
{
    f->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void GtFramebufferObjectBase::Bind()
{
    f->glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void GtFramebufferObjectBase::Release()
{
    f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


GtFramebufferObject::GtFramebufferObject(OpenGLFunctions* f, const SizeI& resolution)
    : GtFramebufferObjectBase(f, resolution)
{

}

void GtFramebufferObject::Create(const GtFramebufferFormat& format)
{
    f->glGenFramebuffers(1, &m_id);
    GtFramebufferObjectBinderScopeGuard binder(this);

    const auto& color_formats = format.m_colorFormats;
    GLenum attachments = GL_COLOR_ATTACHMENT0;

    for(const GtFramebufferTextureFormat& tex_format : color_formats) {
        GtTexture* texture = GtTexture::Create(f, tex_format.m_target, tex_format.m_internalFormat, m_resolution, &tex_format);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, attachments++, texture->GetTarget(), texture->GetId(), 0);
        m_colorAttachments.Push(texture);
    }

    switch (format.m_depthAttachment) {
    case GtFramebufferFormat::RenderBuffer:{
        f->glGenRenderbuffers(1, &m_depthRenderBuffer);
        f->glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
        f->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_resolution.width(), m_resolution.height());
        f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
        break;
    }
    case GtFramebufferFormat::Texture:{
        m_depthTexture = new GtTexture2D(f);
        m_depthTexture->SetInternalFormat(GL_DEPTH_COMPONENT24);
        m_depthTexture->SetSize(m_resolution.width(), m_resolution.height());
        GtTextureFormat depth_format;
        depth_format.PixelFormat = GL_DEPTH_COMPONENT;
        depth_format.PixelType = GL_FLOAT;
        m_depthTexture->SetFormat(depth_format);
        m_depthTexture->Allocate();
        f->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture->GetId(), 0);
        break;
    }
    default:
        break;
    }


    GLuint status = f->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        qCWarning(LC_SYSTEM) << "incomplete framebuffer";
    }
}

GtFramebufferObjectMultisampled::GtFramebufferObjectMultisampled(OpenGLFunctions* f, const SizeI& resolution, quint32 samples)
    : GtFramebufferObjectBase(f, resolution)
    , m_samples(samples)
{
    Q_ASSERT(samples > 1);
}

GtFramebufferObjectMultisampled::~GtFramebufferObjectMultisampled()
{
    f->glDeleteRenderbuffers(m_colorAttachments.Size(), m_colorAttachments.data());
}

void GtFramebufferObjectMultisampled::Create(const GtFramebufferFormat& format)
{
    f->glGenFramebuffers(1, &m_id);
    GtFramebufferObjectBinderScopeGuard binder(this);

    const auto& color_formats = format.m_colorFormats;
    if(!color_formats.IsEmpty()) {
        m_colorAttachments.Resize(color_formats.Size());
        auto it_colors = m_colorAttachments.Begin();

        f->glGenRenderbuffers(color_formats.Size(), it_colors);
        GLenum attachments = GL_COLOR_ATTACHMENT0;

        for(const GtFramebufferTextureFormat& tex_format : color_formats) {
            f->glBindRenderbuffer(GL_RENDERBUFFER, *it_colors);
            f->glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, tex_format.m_internalFormat, m_resolution.width(), m_resolution.height());
            f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachments++, GL_RENDERBUFFER, *it_colors++);
        }
    }

    switch (format.m_depthAttachment) {
    case GtFramebufferFormat::RenderBuffer:{
        f->glGenRenderbuffers(1, &m_depthRenderBuffer);
        f->glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
        f->glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH_COMPONENT16, m_resolution.width(), m_resolution.height());
        f->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
        break;
    }
    case GtFramebufferFormat::Texture:{
        m_depthTexture = new GtTexture2DMultisampled(f, m_samples);
        m_depthTexture->SetInternalFormat(GL_DEPTH_COMPONENT16);
        m_depthTexture->SetSize(m_resolution.width(), m_resolution.height());
        GtTextureFormat depth_format;
        depth_format.PixelFormat = GL_DEPTH_COMPONENT;
        depth_format.PixelType = GL_FLOAT;
        m_depthTexture->SetFormat(depth_format);
        m_depthTexture->Allocate();
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_depthTexture->GetId(), 0);
        break;
    }
    default:
        break;
    }

    GLuint status = f->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        qCWarning(LC_SYSTEM) << "incomplete framebuffer";
    }
}

QImage GtFramebufferObject::ToImage(GLenum colorAttachment) const
{
    if (m_id == 0 || m_resolution.isEmpty()) {
        return QImage();
    }

    GLint prevReadFbo = 0;
    f->glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFbo);

    f->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
    f->glReadBuffer(colorAttachment);

    // Allocate matching image memory
    QImage image(m_resolution.width(), m_resolution.height(), QImage::Format_RGBA8888);

    GLint prevPackAlignment = 4;
    f->glGetIntegerv(GL_PACK_ALIGNMENT, &prevPackAlignment);
    f->glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Read the pixels raw directly into the QImage memory layout
    f->glReadPixels(0, 0, m_resolution.width(), m_resolution.height(),
                    GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    f->glPixelStorei(GL_PACK_ALIGNMENT, prevPackAlignment);
    f->glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFbo);

    return image;
}

// For Multisampled Framebuffers
QImage GtFramebufferObjectMultisampled::ToImage(GLenum colorAttachment) const
{
    if (m_id == 0 || m_resolution.isEmpty()) {
        return QImage();
    }

    GLuint resolveFbo = 0;
    GLuint resolveTex = 0;
    f->glGenFramebuffers(1, &resolveFbo);
    f->glGenTextures(1, &resolveTex);

    f->glBindTexture(GL_TEXTURE_2D, resolveTex);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_resolution.width(), m_resolution.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLint prevDrawFbo = 0, prevReadFbo = 0;
    f->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFbo);
    f->glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFbo);

    f->glBindFramebuffer(GL_FRAMEBUFFER, resolveFbo);
    f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTex, 0);

    f->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_id);
    f->glReadBuffer(colorAttachment);
    f->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFbo);
    GLenum drawBuf = GL_COLOR_ATTACHMENT0;
    f->glDrawBuffers(1, &drawBuf);

    f->glBlitFramebuffer(0, 0, m_resolution.width(), m_resolution.height(),
                         0, 0, m_resolution.width(), m_resolution.height(),
                         GL_COLOR_BUFFER_BIT, GL_NEAREST);

    f->glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFbo);
    f->glReadBuffer(GL_COLOR_ATTACHMENT0);

    QImage image(m_resolution.width(), m_resolution.height(), QImage::Format_RGBA8888);

    GLint prevPackAlignment = 4;
    f->glGetIntegerv(GL_PACK_ALIGNMENT, &prevPackAlignment);
    f->glPixelStorei(GL_PACK_ALIGNMENT, 1);

    f->glReadPixels(0, 0, m_resolution.width(), m_resolution.height(),
                    GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    f->glPixelStorei(GL_PACK_ALIGNMENT, prevPackAlignment);
    f->glBindFramebuffer(GL_READ_FRAMEBUFFER, prevReadFbo);
    f->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFbo);

    f->glDeleteFramebuffers(1, &resolveFbo);
    f->glDeleteTextures(1, &resolveTex);

    return image;
}
