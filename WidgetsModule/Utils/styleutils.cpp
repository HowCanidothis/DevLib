#include "styleutils.h"

#include <QStyle>
#include <QLayout>
#include <QWidget>

#include "widgethelpers.h"

class StyleAdjusterAttachment : public QObject
{
    using Super = QObject;
public:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        switch(event->type()) {
        case QEvent::StyleChange:
        case QEvent::Show:
        case QEvent::LayoutRequest: {
            auto prefSize = watched->property("use_preffered_size");
            if(!prefSize.isNull()) {
                auto* widget = reinterpret_cast<QWidget*>(watched);
                Q_ASSERT(widget->parentWidget() != nullptr);
                auto toSize = prefSize.toSize();
                widget->resize(std::min(toSize.width(), widget->parentWidget()->width()), std::min(toSize.height(), widget->parentWidget()->height()));
            } else {
                reinterpret_cast<QWidget*>(watched)->adjustSize();
            }
        }
            break;
        default:
            break;
        }
        return false;
    }

    static StyleAdjusterAttachment& GetInstance() { static StyleAdjusterAttachment result; return result; }
};

StyleUtils::StyleUtils()
{

}

void StyleUtils::UpdateStyle(class QWidget* target, bool recursive)
{
    auto* style = target->style();
    style->unpolish(target);
    style->polish(target);
    if(recursive) {
        WidgetWrapper(target).ForeachChildWidget([](const WidgetWrapper& widget){
            auto* style = widget->style();
            style->unpolish(widget);
            style->polish(widget);
        });
    }
}

void StyleUtils::ApplyStyleProperty(const char* propertyName, QWidget* target, const QVariant& value, bool recursive)
{
    target->setProperty(propertyName, value);
    UpdateStyle(target, recursive);
}

Q_DECLARE_METATYPE(SharedPointer<QSet<QWidget*>>)

void StyleUtils::InstallSizeAdjuster(QWidget* widget)
{
    widget->installEventFilter(&StyleAdjusterAttachment::GetInstance());
    if(!widget->property("use_preffered_size").isNull()) {
        auto* pw = widget->parentWidget();
        Q_ASSERT(pw != nullptr);
        WidgetWrapper parentWidget(pw);
        auto childs = parentWidget.Injected<QSet<QWidget*>>("childs_to_adjust");
        childs->insert(widget);

        if(!parentWidget->property("has_adjuster_listener").toBool()) {
            parentWidget.AddEventFilter([childs](QObject*, QEvent* e){
                switch (e->type()) {
                case QEvent::Resize:
                    for(QWidget* w : *childs) {
                        auto* layout = w->layout();
                        if(layout != nullptr) {
                            layout->invalidate();
                        }
                    }
                    break;
                default:
                    break;
                }
                return false;
            });
            parentWidget->setProperty("has_adjuster_listener", true);
        }
        QObject::connect(widget, &QWidget::destroyed, [widget, childs]{
            childs->remove(widget);
        });
    }
}
