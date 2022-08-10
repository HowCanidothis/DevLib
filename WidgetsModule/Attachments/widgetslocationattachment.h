#ifndef WIDGETSLOCATIONATTACHMENT_H
#define WIDGETSLOCATIONATTACHMENT_H

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/widgetsdeclarations.h"

class WidgetsLocationAttachment : public QObject
{
    using Super = QObject;
    WidgetsLocationAttachment(QWidget* target, const DescWidgetsLocationAttachmentParams& params = DescWidgetsLocationAttachmentParams());
public:
    class ComponentPlacer* GetComponentPlacer() { return m_componentPlacer.get(); }

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    friend class ObjectWrapper;
    friend class WidgetWrapper;
    ScopedPointer<ComponentPlacer> m_componentPlacer;
    QWidget* m_target;
    QWidget* m_parent;
};

#endif // WIDGETSLOCATIONATTACHMENT_H
