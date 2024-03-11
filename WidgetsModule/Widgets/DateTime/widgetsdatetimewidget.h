#ifndef WIDGETSDATETIMEWIDGET_H
#define WIDGETSDATETIMEWIDGET_H

#include <QFrame>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsDateTimeWidget;
}

class WidgetsDateTimeWidget : public QFrame
{
	Q_OBJECT
	using Super = QFrame;
public:
    enum Mode {
        DateTime,
        Date,
        Time
    };

    explicit WidgetsDateTimeWidget(QWidget *parent = nullptr);
    ~WidgetsDateTimeWidget() override;
	
    DispatcherConnections ConnectModel(const char* locationInfo, LocalPropertyDateTime* modelProperty);
    DispatcherConnections ConnectModel(const char* locationInfo, LocalPropertyDate* modelProperty);

    LocalPropertyInt Mode;
    LocalPropertyBool NowEnabled;
    LocalPropertyDateTime CurrentDateTime;
    LocalPropertyDoubleOptional TimeShift;

	Dispatcher OnNowActivate;
	Dispatcher OnApplyActivate;
    Dispatcher Store;
    Dispatcher Reset;
	
    Dispatcher OnAboutToShow;
protected:
    void showEvent(QShowEvent* event) override;
private:
    Ui::WidgetsDateTimeWidget *ui;
    DispatcherConnectionsSafe m_connections;
};

#endif // WIDGETSDATETIMEWIDGET_H
