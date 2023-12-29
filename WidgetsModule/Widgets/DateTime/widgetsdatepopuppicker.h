#ifndef WIDGETSDATEPOPUPPICKER_H
#define WIDGETSDATEPOPUPPICKER_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsDatePopupPicker;
}

class WidgetsDatePopupPicker : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetsDatePopupPicker(QWidget *parent = nullptr);
    ~WidgetsDatePopupPicker();

    class WidgetsDateTimeWidget* GetDateWidget() const { return m_editor; }
    class WidgetsDateTimeEdit* GetLineEdit() const;

    Dispatcher OnCloseEditor;
    Dispatcher OnDataCommit;

    LocalPropertyBool ForceDisabled;

private:
    Ui::WidgetsDatePopupPicker *ui;
    WidgetsDateTimeWidget* m_editor;
    DispatcherConnectionsSafe m_connections;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
};

#endif // WIDGETSDATEPOPUPPICKER_H
