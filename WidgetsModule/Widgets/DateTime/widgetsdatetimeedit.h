#ifndef WIDGETSDATETIMEEDIT_H
#define WIDGETSDATETIMEEDIT_H

#include <QDateTimeEdit>

#include <PropertiesModule/internal.hpp>

class WidgetsDateTimeEdit : public QDateTimeEdit
{
    using Super = QDateTimeEdit;
public:
//    using Super::Super;
    WidgetsDateTimeEdit(QWidget* parent = nullptr);

    LocalPropertyBool IsValid;
    LocalPropertyLocale Locale;
    LocalPropertyString DisplayFormat;

    QDateTime dateTimeFromText(const QString &text) const override;
    QString textFromDateTime(const QDateTime &dt) const override;
    QValidator::State validate(QString&, int&) const override { return QValidator::Acceptable; }
};

class LocalPropertiesWidgetsDateTimeConnector : public LocalPropertiesWidgetConnectorBase
{
    using Super = LocalPropertiesWidgetConnectorBase;
public:
    LocalPropertiesWidgetsDateTimeConnector(LocalPropertyDateTime* property, WidgetsDateTimeEdit* dateTime);
};


#endif // WIDGETSDATETIMEEDIT_H
