#ifndef GTELLIPSELINEDRAWABLE_H
#define GTELLIPSELINEDRAWABLE_H

#include "Objects/gtobjectbase.h"

class GtEllipseLineDrawable : public GtDrawableBase
{
    using Super = GtDrawableBase;
public:
    GtEllipseLineDrawable(GtRenderer* renderer, const GtShaderProgramPtr& shaderProgram);

    std::atomic_bool AutoUpdateTransform;
    LocalProperty<Vector3F> Up;
    LocalProperty<Vector3F> Normal;
    LocalProperty<Point3F> Position;
    LocalPropertyColor Color;
    LocalProperty<SizeF> Radius;
    LocalProperty<QMatrix4x4> Transform;
    LocalPropertyFloat Width;
    LocalPropertyBool Visible;

private:
    GtMeshBufferPtr m_buffer;
    ScopedPointer<class GtMaterial> m_material;

    // GtDrawableBase interface
protected:
    void drawDepth(OpenGLFunctions* ) override;
    void draw(OpenGLFunctions* f) override;
    void onInitialize(OpenGLFunctions* f) override;
    void onDestroy(OpenGLFunctions*) override;

private:
    DelayedCallObject m_recompute;
};
#endif // GTELLIPSELINEDRAWABLE_H
