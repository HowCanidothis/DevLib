#ifndef GTSCENE_H
#define GTSCENE_H

#include <SharedModule/internal.hpp>
#include <SharedGuiModule/internal.hpp>

class GtInteractableBase;
class GtDrawableBase;

class GtScene
{
    QSet<GtInteractableBase*> m_interactables;
    QSet<GtDrawableBase*> m_drawables;

public:
    using FInitializationFunction = std::function<void (OpenGLFunctions*)>;
    GtScene();
    ~GtScene();


    Stack<GtInteractableBase*> FindClosestToPoint(const Point3F& point, float tolerance) const;

    void AddInteractable(GtInteractableBase* interactable);
    void AddDrawable(GtDrawableBase* drawable);

    void RemoveInteractable(GtInteractableBase* interactable);
    void RemoveDrawable(GtDrawableBase* drawable);

private:
    friend class GtView;
    void initialize(OpenGLFunctions* f);
    void draw(OpenGLFunctions* f);

    void foreachGtDrawableBase(const std::function<void (GtDrawableBase*)>& action);

private:
    FInitializationFunction m_initFunction;
    bool m_initialized;
};

#endif // GTSCENE_H
