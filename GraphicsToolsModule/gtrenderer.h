#ifndef GTRENDERER_H
#define GTRENDERER_H

#include "gtrendererbase.h"

class GtRenderer : public GtRendererBase
{
public:
    GtRenderer();

    void initialize() override;
    void resize(qint32 x, qint32 y) override;
    void paint() override;
};

#endif // GTRENDERER_H
