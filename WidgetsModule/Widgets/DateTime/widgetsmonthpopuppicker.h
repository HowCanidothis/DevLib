#ifndef WIDGETSMONTHPOPUPPICKER_H
#define WIDGETSMONTHPOPUPPICKER_H

#include <QWidget>
#include <WidgetsModule/internal.hpp>

namespace Ui {
class WidgetsMonthPopupPicker;
}

class WidgetsMonthPicker;
class WidgetsMonthPopupPicker : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetsMonthPopupPicker(QWidget *parent = nullptr);
    ~WidgetsMonthPopupPicker();

    LocalPropertyDate Date;
    LocalPropertyBool Enable;
    LocalPropertyBool ShowLeftRight;
    WidgetsMonthPicker* GetPopup() { return m_edit; }
    WidgetsDateTimeEdit* GetDateEdit() const;

    const QDate& GetDate() const;
    void SetDate(const QDate& d);

    DispatcherConnections ConnectModel(const char* locationInfo, LocalPropertyDate& model);

    void ShowPopup();
    void ClosePopup();
    Dispatcher OnCloseEditor;

private:
    Ui::WidgetsMonthPopupPicker *ui;
    QMenu* m_menu;
    WidgetsMonthPicker* m_edit;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
};

#endif // WIDGETSMONTHPOPUPPICKER_H
