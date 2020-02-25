#ifndef GTRENDERERBASE_H
#define GTRENDERERBASE_H

#include <SharedGuiModule/internal.hpp>

class GtRendererBase : public OpenGLFunctions
{
public:
    GtRendererBase();

    virtual void initialize() = 0;
    virtual void resize(qint32 x, qint32 y) = 0;
    virtual void paint() = 0;
};

#endif // GTRENDERERBASE_H
