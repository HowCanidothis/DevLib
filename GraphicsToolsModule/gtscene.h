#ifndef GTSCENE_H
#define GTSCENE_H

#include <SharedModule/internal.hpp>
#include <SharedGuiModule/internal.hpp>

class GtInteractableBase;
class GtDrawableBase;

class GtScene
{
    QSet<GtDrawableBase*> m_drawables;

public:
    using FInitializationFunction = std::function<void (OpenGLFunctions*)>;
    GtScene();
    ~GtScene();

    void AddDrawable(GtDrawableBase* drawable);
    void RemoveDrawable(GtDrawableBase* drawable);

private:
    friend class GtView;
    friend class GtRenderer;
    void draw(OpenGLFunctions* f);

private:
    FInitializationFunction m_initFunction;
    bool m_initialized;
};

#endif // GTSCENE_H
