#ifndef FLOATINGWIDGETLOCATIONATTACHMENT_H
#define FLOATINGWIDGETLOCATIONATTACHMENT_H

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

class FloatingWidgetLocationAttachment : public QObject
{
public:
    FloatingWidgetLocationAttachment(class QWidget* target, QuadTreeF::BoundingRect_Location location, const QPoint& offset);
    static void Attach(QWidget* target, QuadTreeF::BoundingRect_Location location, const QPoint& offset) { new FloatingWidgetLocationAttachment(target, location, offset); }

    class ComponentPlacer* GetComponentPlacer() { return m_componentPlacer.get(); }

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    ScopedPointer<ComponentPlacer> m_componentPlacer;
    QWidget* m_target;
};

#endif // FLOATINGWIDGETLOCATIONATTACHMENT_H
