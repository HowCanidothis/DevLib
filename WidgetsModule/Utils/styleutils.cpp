#include "styleutils.h"

#include <QStyle>
#include <QWidget>

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

void StyleUtils::ApplyStyleProperty(const char* propertyName, QWidget* target, const QVariant& value)
{
    target->setProperty(propertyName, value);
    auto* style = target->style();
    style->unpolish(target);
    style->polish(target);
}

void StyleUtils::InstallSizeAdjuster(QWidget* widget)
{
    widget->installEventFilter(&StyleAdjusterAttachment::GetInstance());
}
