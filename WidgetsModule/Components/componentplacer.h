#ifndef COMPONENTPLACER_H
#define COMPONENTPLACER_H

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

class ComponentPlacer
{
public:
    ComponentPlacer(const DelayedCallObjectParams& params = DelayedCallObjectParams());

    void Initialize();

    LocalProperty<QPoint> ResultPosition;
    LocalProperty<QSize> ParentSize;
    LocalProperty<QSize> TargetSize;
    LocalProperty<QuadTreeF::BoundingRect_Location> Location;
    LocalProperty<QPoint> Offset;

private:
    void updateLocator();
    void updateLocation();
    qint32 targetWidth() const { return TargetSize.Native().width(); }
    qint32 targetHeight() const { return TargetSize.Native().height(); }
    qint32 width() const { return ParentSize.Native().width(); }
    qint32 height() const { return ParentSize.Native().height(); }
    qint32 offsetX() const { return Offset.Native().x(); }
    qint32 offsetY() const { return Offset.Native().y(); }

private:
    FAction m_locator;
    DelayedCallObject m_locate;
};

#endif // COMPONENTPLACER_H
