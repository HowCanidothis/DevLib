#ifndef GTRENDERERCONTROLLER_H
#define GTRENDERERCONTROLLER_H

#include <SharedModule/internal.hpp>
#include <SharedGuiModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

#include "decl.h"

class GtCameraAnimationEngine
{
    template<class ValueType> friend class GtCameraPointsPathEngineBase;
public:
    GtCameraAnimationEngine(class GtRenderer* renderer, class GtCamera* camera);

    void SetRotationCurve(const QEasingCurve& curve) { m_rotationCurve = curve; }
    void SetMovementCurve(const QEasingCurve& curve) { m_movementCurve = curve; }
    void SetRotationDuration(qint32 msecs) { m_rotationDuration = msecs; }
    void SetMovementDuration(qint32 msecs) { m_movementDuration = msecs; }

    void Move(const Point3F& center, const Vector3F& forward, const Vector3F& up, float distance);
    void Move(const Point3F& center, float distance);
    void Move(const Point3F& newEye);

    bool IsRunning() const;

private:
    GtCamera* m_camera;
    GtRenderer* m_renderer;
    ScopedPointer<class QParallelAnimationGroup> m_animation;
    DispatcherConnectionsSafe m_connections;
    qint32 m_rotationDuration;
    qint32 m_movementDuration;
    QEasingCurve m_rotationCurve;
    QEasingCurve m_movementCurve;
    mutable QMutex m_mutex;
};

template<class Container>
class GtCameraPointsPathEngineBase
{
public:
    using Extractor = std::function<Point3F (const typename Container::value_type& valueType)>;
    GtCameraPointsPathEngineBase(GtCameraAnimationEngine* animationEngine,  const Extractor& pointExtractor)
        : CurrentIndex(-1)
        , m_animationEngine(animationEngine)
        , m_pointExtractor(pointExtractor)
    {
        CurrentIndex.SetMinMax(-1, -1);
        m_previousIndex.ConnectFrom(CurrentIndex);
    }

    void Clear()
    {
        m_points.clear();
        CurrentIndex.SetMinMax(-1,-1);
    }

    void SetPoints(const Container& points)
    {
        m_points = points;
        CurrentIndex.SetMinMax(points.isEmpty() ? -1 : 0, points.size() - 1);
        CurrentIndex = 0;
    }
    const Container& GetPoints() const { return m_points; }
    Point3F GetCurrentPoint() const { return CurrentIndex == -1 ? Point3F() : extractPoint(CurrentIndex); }
    GtCameraAnimationEngine* GetAnimationEngine() const { return m_animationEngine; }

    LocalPropertyInt CurrentIndex;

protected:
    Point3F extractPoint(qint32 index) const
    {
        const auto& value = m_points.at(index);
        return m_pointExtractor(value);
    }
    GtCamera* getCamera() const { return m_animationEngine->m_camera; }

protected:
    GtCameraAnimationEngine* m_animationEngine;
    Container m_points;
    LocalPropertyInt m_previousIndex;
    Extractor m_pointExtractor;
};

template<class Container>
class GtCameraPointsPathEngine : public GtCameraPointsPathEngineBase<Container>
{
    using Super = GtCameraPointsPathEngineBase<Container>;
public:
    using Extractor = typename Super::Extractor;
    GtCameraPointsPathEngine(GtCameraAnimationEngine* animationEngine,  const Extractor& extractor, float defaultDistance = 100.f)
        : Super(animationEngine, extractor)
        , m_defaultDistance(defaultDistance)
    {
        Super::CurrentIndex.Subscribe([this]{
            if(Super::CurrentIndex != -1) {
                float currentDistance = m_defaultDistance;
                if(Super::m_previousIndex != -1) {
                    currentDistance = extractPoint(Super::m_previousIndex).distanceToPoint(Super::m_animationEngine->m_camera->GetEye());
                }
                Super::m_animationEngine->Move(extractPoint(Super::CurrentIndex), currentDistance);
            }
        });
    }

    void SetPoints(const Container& points)
    {
        float currentDistance = m_defaultDistance;
        if(Super::CurrentIndex != -1) {
            currentDistance = extractPoint(Super::CurrentIndex).distanceToPoint(Super::m_animationEngine->m_camera->GetEye());
        }

        Super::m_points = points;
        Super::CurrentIndex.SetMinMax(points.isEmpty() ? -1 : 0, points.size() - 1);
        Super::CurrentIndex = 0;
        if(Super::CurrentIndex != -1) {
            Super::m_animationEngine->Move(extractPoint(Super::CurrentIndex), currentDistance);
        }
    }

private:
    float m_defaultDistance;
};

template<class Container>
class GtCameraPointsPathFixedDistanceEngine : public GtCameraPointsPathEngineBase<Container>
{
    using Super = GtCameraPointsPathEngineBase<Container>;
public:
    using Extractor = typename Super::Extractor;
    GtCameraPointsPathFixedDistanceEngine(GtCameraAnimationEngine* animationEngine,  const Extractor& extractor)
        : Super(animationEngine, extractor)
    {
        Super::CurrentIndex.Subscribe([this]{
            if(Super::CurrentIndex != -1) {
                Super::m_animationEngine->Move(extractPoint(Super::CurrentIndex) - m_direction);
            }
        });
    }

    void UpdateDirection()
    {
        if(Super::CurrentIndex != -1) {
            m_direction = extractPoint(Super::CurrentIndex) - Super::getCamera()->GetEye();
        }
    }

    void SetPoints(const Container& points, qint32 currentIndex)
    {
        Q_ASSERT(currentIndex != -1 && !points.isEmpty());
        Super::m_points = points;
        m_direction = Super::extractPoint(currentIndex) - Super::getCamera()->GetEye();
        Super::CurrentIndex.SetMinMax(0, points.size() - 1);
        Super::CurrentIndex = currentIndex;
    }

private:
    Vector3F m_direction;
};

class GtRendererController : public QObject
{
    Q_OBJECT
public:
    GtRendererController(GtRenderer* renderer, class ControllersContainer* controllersContainer, struct GtControllersContext* context);
    ~GtRendererController();

    void UpdateFrame();

    template<class T, typename ...Args>
    T* CreateDrawable(Args... args)
    {
        return CreateDrawableQueued<T, Args...>(0, args...);
    }

    template<class T, typename ...Args>
    T* CreateDrawableQueued(qint32 queueNumber, Args... args)
    {
        auto result = new T(m_renderer, args...);
        result->m_rendererDrawable = true;
        m_renderer->Asynch([this, result]{
            result->initialize(m_renderer);
        });
        m_drawables[queueNumber].append(result);
        return result;
    }

    void RemoveDrawable(qint32 queueNumber, GtDrawableBase* drawable);
    void ClearQueue(qint32 queueNumber);

    void SetRenderPath(const GtRenderPathPtr& renderPath);
    void SetRenderProperties(const GtRenderProperties& renderProperties);
    void SetRenderProperty(const Name& name, const QVariant& value);
    void SetProjectionProperties(float angle, float nearValue, float farValue);

    void Resize(qint32 w, qint32 h);
    void MouseMoveEvent(QMouseEvent* event);
    void MousePressEvent(QMouseEvent* event);
    void MouseReleaseEvent(QMouseEvent* event);
    void WheelEvent(QWheelEvent* event);
    void KeyPressEvent(QKeyEvent* event);
    void KeyReleaseEvent(QKeyEvent* event);

    LocalPropertyColor SpaceColor;
    LocalPropertyBool Enabled;

    GtCameraAnimationEngine& GetCameraAnimationEngine() { return m_cameraAnimationEngine; }
    GtCamera* GetCamera() { return m_camera.get(); }
    QImage GetCurrentImage() const;
    double GetRenderTime() const { return m_renderTime; }
    const SizeF& GetVisibleSize() const { return m_visibleSize; }

    Dispatcher OnAboutToBeDestroyed;

signals:
    void imageUpdated();

private:
    bool isDirtyReset();
    void destroyDrawable(GtDrawableBase* drawable);
    void calculateVisibleSize();
    void drawSpace(OpenGLFunctions* f);
    void draw(OpenGLFunctions* f);
    void drawDepth(OpenGLFunctions* f);
    void setCurrentImage(QImage* image, double renderTime);
    void onInitialize();
    void onDestroy();
    const GtRenderProperties& getRenderProperties() const { return m_renderProperties; }

private:
    friend class GtRenderer;
    mutable QMutex m_outputImageMutex;
    GtRenderer* m_renderer;
    ScopedPointer<QImage> m_outputImage;
    ScopedPointer<GtCamera> m_camera;
    ScopedPointer<GtControllersContext> m_controllersContext;
    ScopedPointer<ControllersContainer> m_controllers;

    ScopedPointer<class GtFramebufferObjectBase> m_depthFbo;
    ScopedPointer<class QOpenGLFramebufferObject> m_fbo;
    ScopedPointer<class QOpenGLFramebufferObject> m_nonTransparent;
    ScopedPointer<class QOpenGLFramebufferObject> m_transparent;
    ScopedPointer<class QOpenGLFramebufferObject> m_transparentAlpha;
    double m_renderTime;
    GtCameraAnimationEngine m_cameraAnimationEngine;
    GtRenderProperties m_renderProperties;
    QMap<qint32, QVector<GtDrawableBase*>> m_drawables;
    DispatcherConnectionsSafe m_connections;
    SizeF m_visibleSize;
    DelayedCallObject m_resize;
    bool m_dirty;
    GtRenderPathPtr m_renderPath;
};

#endif // GTRENDERERCONTROLLER_H
