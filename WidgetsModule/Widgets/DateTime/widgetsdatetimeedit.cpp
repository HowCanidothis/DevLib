#include "widgetsdatetimeedit.h"

#include <QKeyEvent>

#include "WidgetsModule/Utils/styleutils.h"
#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsDateTimeEdit::WidgetsDateTimeEdit(QWidget* parent)
    : Super(parent)
    , IsValid(true)
{
    IsValid.SetAndSubscribe([this]{
        setReadOnly(!IsValid);

        if(!IsValid) {
            setDisplayFormat(displayFormat());
        }
        StyleUtils::UpdateStyle(this);
    });

    WidgetsAttachment::Attach(this, [this](QObject*, QEvent* event){
        if(event->type() == QEvent::KeyPress) {
            auto keyEvent = reinterpret_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Delete) {
                IsValid = false;
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
    return IsValid ? Super::dateTimeFromText(text) : QDateTime();
}

QString WidgetsDateTimeEdit::textFromDateTime(const QDateTime& dt) const
{
    return IsValid ? Super::textFromDateTime(dt) : "";
}

LocalPropertiesWidgetsDateTimeConnector::LocalPropertiesWidgetsDateTimeConnector(LocalPropertyDateTime* property, WidgetsDateTimeEdit* dateTime)
 : Super([dateTime, property](){
            dateTime->setDateTime(*property);
        },
        [dateTime, property](){
            *property = dateTime->IsValid ? dateTime->dateTime() : QDateTime();
        }
)
{
    property->GetDispatcher().Connect(this, [this]{
        m_widgetSetter();
    }).MakeSafe(m_dispatcherConnections);

    m_connections.connect(dateTime, &QDateTimeEdit::dateTimeChanged, [this](){
        m_propertySetter();
    });
}
