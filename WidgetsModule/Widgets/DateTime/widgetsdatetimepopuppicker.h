#ifndef WIDGETSDATETIMEPOPUPPICKER_H
#define WIDGETSDATETIMEPOPUPPICKER_H

#include <QFrame>
#include <QDateTimeEdit>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsDatetimePopupPicker;
}

class WidgetsDatetimePopupPicker : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
public:
    explicit WidgetsDatetimePopupPicker(QWidget *parent = nullptr);
    ~WidgetsDatetimePopupPicker() override;

    class WidgetsDateTimeWidget* GetDateTimeWidget() const { return m_editor; }
    class WidgetsDateTimeEdit* GetLineEdit() const;

    LocalPropertyString DisplayFormat;
    LocalPropertyLocale Locale;
        
    Dispatcher OnCloseEditor;
    Dispatcher OnDataCommit;

private:
    Ui::WidgetsDatetimePopupPicker *ui;
    WidgetsDateTimeWidget* m_editor;
    DispatcherConnectionsSafe m_connections;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
};

#endif // WIDGETSDATETIMEPOPUPPICKER_H
