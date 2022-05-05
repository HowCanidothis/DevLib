#include "styleutils.h"

#include <QStyle>
#include <QWidget>

#include "widgethelpers.h"

class StyleAdjusterAttachment : public QObject
{
    using Super = QObject;
public:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if(event->type() == QEvent::StyleChange || event->type() == QEvent::Show) {
            reinterpret_cast<QWidget*>(watched)->adjustSize();
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
        WidgetWrapper(target).ForeachChildWidget([](QWidget* widget){
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

void StyleUtils::InstallSizeAdjuster(QWidget* widget)
{
    widget->installEventFilter(&StyleAdjusterAttachment::GetInstance());
}
