#ifndef WIDGETSLOCATIONATTACHMENT_H
#define WIDGETSLOCATIONATTACHMENT_H

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

struct DescWidgetsLocationAttachmentParams
{
    DescWidgetsLocationAttachmentParams(QWidget* target, QuadTreeF::BoundingRect_Location location)
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

    DescWidgetsLocationAttachmentParams& SetOffset(const QPoint& offset) { Offset = offset; return *this; }
    DescWidgetsLocationAttachmentParams& SetDelay(qint32 delay) { Delay = delay; return *this; }
    DescWidgetsLocationAttachmentParams& SetRelativeParent(QWidget* parent) { RelativeParent = parent; return *this; }
};

class WidgetsLocationAttachment : public QObject
{
    using Super = QObject;
    WidgetsLocationAttachment(const DescWidgetsLocationAttachmentParams& params);
public:       
    static WidgetsLocationAttachment* Attach(const DescWidgetsLocationAttachmentParams& params)
    {
        return new WidgetsLocationAttachment(params); // Note: target takes ownership of the attachment
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

#endif // WIDGETSLOCATIONATTACHMENT_H
