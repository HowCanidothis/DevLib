#ifndef GTLINESDRAWABLE_H
#define GTLINESDRAWABLE_H

#include "Objects/gtobjectbase.h"

class GtLinesDrawable : public GtDrawableBase
{
    using Super = GtDrawableBase;
public:
    GtLinesDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram);

    LocalPropertyVector<Point3F> Points;
    LocalPropertyColor Color;
    LocalProperty<QMatrix4x4> Transform;
    LocalPropertyInt Visible;
    LocalPropertyFloat Width;

    // GtDrawableBase interface
protected:
    void drawDepth(OpenGLFunctions* f) override;
    void draw(OpenGLFunctions* f) override;
    void onInitialize(OpenGLFunctions* f) override;
    void onDestroy(OpenGLFunctions* f) override;
    void onAboutToDestroy() override {}

private:
    GtMeshBufferPtr m_buffer;
    ScopedPointer<class GtMaterial> m_material;
};

#endif // GTLINESDRAWABLE_H
