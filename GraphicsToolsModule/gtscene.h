#ifndef GTSCENE_H
#define GTSCENE_H

#include <SharedModule/internal.hpp>
#include <SharedGuiModule/internal.hpp>

class GtInteractableBase;
class GtDrawableBase;

class GtScene
{
    QMap<qint32, QSet<GtDrawableBase*>> m_drawables;

public:
    using FInitializationFunction = std::function<void (OpenGLFunctions*)>;
    GtScene();
    ~GtScene();

    void AddDrawable(GtDrawableBase* drawable, qint32 queueNumber);
    void RemoveDrawable(GtDrawableBase* drawable);

private:
    friend class GtView;
    friend class GtRenderer;
    void draw(OpenGLFunctions* f);
    void drawDepth(OpenGLFunctions* f);

private:
    FInitializationFunction m_initFunction;
    bool m_initialized;
};

#endif // GTSCENE_H
