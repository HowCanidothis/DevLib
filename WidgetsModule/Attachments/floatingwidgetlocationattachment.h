#ifndef FLOATINGWIDGETLOCATIONATTACHMENT_H
#define FLOATINGWIDGETLOCATIONATTACHMENT_H

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

class FloatingWidgetLocationAttachment : public QObject
{
public:
    FloatingWidgetLocationAttachment(class QWidget* target, QuadTreeF::BoundingRect_Location location, const QPoint& offset, QWidget* relativeWidget = nullptr, qint32 delay = 500);
    static void Attach(QWidget* target, QuadTreeF::BoundingRect_Location location, const QPoint& offset, QWidget* relativeWidget = nullptr) { new FloatingWidgetLocationAttachment(target, location, offset, relativeWidget); }

    class ComponentPlacer* GetComponentPlacer() { return m_componentPlacer.get(); }

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    ScopedPointer<ComponentPlacer> m_componentPlacer;
    QWidget* m_target;
    QWidget* m_parent;
};

#endif // FLOATINGWIDGETLOCATIONATTACHMENT_H
