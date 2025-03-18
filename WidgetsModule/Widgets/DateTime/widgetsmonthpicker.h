#ifndef WIDGETSMONTHPICKER_H
#define WIDGETSMONTHPICKER_H

#include <QFrame>
#include <WidgetsModule/internal.hpp>

namespace Ui {
class WidgetsMonthPicker;
}

class WidgetsMonthPicker : public QFrame
{
    Q_OBJECT

public:
    explicit WidgetsMonthPicker(QWidget *parent = nullptr);
    ~WidgetsMonthPicker();

    DispatcherConnections ConnectModel(const char* locationInfo, LocalPropertyDate* model);
    DispatcherConnections ConnectModel(const char* locationInfo, LocalPropertyInt* month, LocalPropertyInt* year);

    LocalPropertyBool EnableButtons;
    LocalPropertyDate Date;

    Dispatcher OnApply;
    Dispatcher OnCancel;
private:
    Ui::WidgetsMonthPicker *ui;
    LocalPropertyInt m_buttonIndex;
    QVector<QPushButton*> m_currButtons;
    DispatcherConnectionsSafe m_connections;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
};

#endif // WIDGETSMONTHPICKER_H
