#include "gtframetexture.h"

#ifdef OPENCV

#include <opencv2/opencv.hpp>

GtFrameTexture::GtFrameTexture(OpenGLFunctions* f)
    : f(f)
    , input(nullptr)
{
}

GtFrameTexture::~GtFrameTexture()
{

}

void GtFrameTexture::createOutput()
{
    if(output_texture == nullptr) {
        output_texture.reset(new GtTexture2D(f));
        output_texture->Create();
    }
}

void GtFrameTexture::setInput(const cv::Mat* input)
{
    m_glFormat.Pixels = input->data;
    if(!this->input || (input->type() != cv_type)) {
        gTexInternalFormat internal = cv2glInternalFormat(input->type());
        output_texture->SetInternalFormat(internal);
    }

    QSize size = output_texture->GetSize();
    if(size.width() != input->cols || size.height() != input->rows) {
        output_texture->SetSize(input->cols, input->rows);
    }

    this->input = input;
}

const GtTexture2D* GtFrameTexture::getOutput() const
{
    return output_texture.data();
}

void GtFrameTexture::update()
{
    output_texture->Allocate(m_glFormat);
}

gTexInternalFormat GtFrameTexture::cv2glInternalFormat(qint32 type)
{
    cv_type = type;
    switch (type) {
    case CV_8UC1:
        m_glFormat.PixelFormat = GL_RED_INTEGER;
        m_glFormat.PixelType = GL_UNSIGNED_BYTE;
        return GL_R8UI;
    case CV_16UC1:
        m_glFormat.PixelFormat = GL_RED_INTEGER;
        m_glFormat.PixelType = GL_UNSIGNED_SHORT;
        return GL_R16UI;
    default:
        Q_ASSERT_X(false, "GtFrameTexture::cv2glInternalFormat", "Invalid format");
        break;
    }
    return 0;
}

#endif
