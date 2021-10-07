#include "widgetsdatetimeedit.h"

#include <QKeyEvent>

#include "WidgetsModule/Utils/styleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDateTimeEdit::WidgetsDateTimeEdit(QWidget* parent)
    : Super(parent)
    , m_recursionBlock(false)
{
    CurrentDateTime.SetAndSubscribe([this]{
        if(m_recursionBlock) {
            return;
        }
        guards::LambdaGuard guard([this]{ m_recursionBlock = false; }, [this]{ m_recursionBlock = true; });
        setReadOnly(CurrentDateTime.IsRealTime());

        setDisplayFormat(displayFormat());
        setDateTime(CurrentDateTime);
        StyleUtils::UpdateStyle(this);
    });

    setButtonSymbols(WidgetsDateTimeEdit::NoButtons);

    CurrentDateTime.OnMinMaxChanged.Connect(this, [this]{
        setDateTimeRange(CurrentDateTime.GetMinValid(), CurrentDateTime.GetMaxValid());
    });

    connect(this, &WidgetsDateTimeEdit::dateTimeChanged, [this]{
        if(m_recursionBlock) {
            return;
        }
        guards::LambdaGuard guard([this]{ m_recursionBlock = false; }, [this]{ m_recursionBlock = true; });
        CurrentDateTime = dateTime();
    });

    WidgetsAttachment::Attach(this, [this](QObject*, QEvent* event){
        if(event->type() == QEvent::KeyPress) {
            auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Delete) {
                CurrentDateTime = QDateTime();
                return true;
            }
        }
        return false;
    });

    Locale.Subscribe([this]{
        setLocale(Locale);
    });

    DisplayFormat.Subscribe([this]{
        setDisplayFormat(DisplayFormat);
    });
}

QDateTime WidgetsDateTimeEdit::dateTimeFromText(const QString& text) const
{
    return !CurrentDateTime.IsRealTime() ? Super::dateTimeFromText(text) : QDateTime();
}

QString WidgetsDateTimeEdit::textFromDateTime(const QDateTime& dt) const
{
    return !CurrentDateTime.IsRealTime() ? Super::textFromDateTime(dt) : "";
}
