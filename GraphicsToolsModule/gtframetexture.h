#ifndef DEPTHSMOOTHCOMPUTENODE_H
#define DEPTHSMOOTHCOMPUTENODE_H

#ifdef OPENCV

#include "SharedGuiModule/decl.h"
#include "gttexture2D.h"

namespace cv {
    class Mat;
}

class GtFrameTexture
{
    qint32 cv_type=-1;

    OpenGLFunctions* f;
    ScopedPointer<GtTexture2D> output_texture;
    const cv::Mat* input;
    GtTextureFormat gl_format;

    gLocID loc_sampler0;
    gLocID loc_resolution;
public:
    GtFrameTexture(OpenGLFunctions* f);
    ~GtFrameTexture();

    void createOutput();

    void setInput(const cv::Mat* input);
    const GtTexture2D* getOutput() const;
    GtTexture2D* getOutput() { return output_texture.data(); }

    void update();
    // GtComputeNodeBase interface
protected:
    bool onInputChanged(const cv::Mat* input);
    void update(const cv::Mat* input);
    gTexInternalFormat cv2glInternalFormat(qint32 type);
};

#endif

#endif // DEPTHSMOOTHCOMPUTENODE_H
