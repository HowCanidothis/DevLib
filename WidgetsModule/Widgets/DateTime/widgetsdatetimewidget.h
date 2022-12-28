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
        Date
    };

    explicit WidgetsDateTimeWidget(QWidget *parent = nullptr);
    ~WidgetsDateTimeWidget() override;
	
    DispatcherConnections ConnectModel(LocalPropertyDateTime* modelProperty);
    DispatcherConnections ConnectModel(LocalPropertyDate* modelProperty);

    LocalPropertyInt Mode;
    LocalPropertyBool NowEnabled;
    LocalPropertyDateTime CurrentDateTime;

	Dispatcher OnNowActivate;
	Dispatcher OnApplyActivate;
    Dispatcher Store;
    Dispatcher Reset;
	
private:
    Ui::WidgetsDateTimeWidget *ui;
    DispatcherConnectionsSafe m_connections;
};

#endif // WIDGETSDATETIMEWIDGET_H
