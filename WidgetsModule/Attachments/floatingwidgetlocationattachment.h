#ifndef FLOATINGWIDGETLOCATIONATTACHMENT_H
#define FLOATINGWIDGETLOCATIONATTACHMENT_H

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

struct DescFloatingWidgetLocationAttachmentParams
{
    DescFloatingWidgetLocationAttachmentParams(QWidget* target, QuadTreeF::BoundingRect_Location location)
        : Target(target)
        , Location(location)
        , Delay(500)
        , RelativeParent(nullptr)
    {}

    QWidget* Target;
    QuadTreeF::BoundingRect_Location Location;
    qint32 Delay;
    QWidget* RelativeParent;
    QPoint Offset;

    DescFloatingWidgetLocationAttachmentParams& SetOffset(const QPoint& offset) { Offset = offset; return *this; }
    DescFloatingWidgetLocationAttachmentParams& SetDelay(qint32 delay) { Delay = delay; return *this; }
    DescFloatingWidgetLocationAttachmentParams& SetRelativeParent(QWidget* parent) { RelativeParent = parent; return *this; }
};

class FloatingWidgetLocationAttachment : public QObject
{
    using Super = QObject;
    FloatingWidgetLocationAttachment(const DescFloatingWidgetLocationAttachmentParams& params);
public:       
    static FloatingWidgetLocationAttachment* Attach(const DescFloatingWidgetLocationAttachmentParams& params)
    {
        return new FloatingWidgetLocationAttachment(params); // Note: target takes ownership of the attachment
    }

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
