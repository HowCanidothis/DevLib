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
	
    DispatcherConnections ConnectModel(LocalPropertyDateTime* modelProperty, bool reactive = true);
    DispatcherConnections ConnectModel(LocalPropertyDate* modelProperty, bool reactive = true);

    LocalPropertyInt Mode;
    LocalPropertyBool NowEnabled;
    LocalPropertyDateTime CurrentDateTime;
    LocalPropertyLocale Locale;

	Dispatcher OnNowActivate;
	Dispatcher OnApplyActivate;
    Dispatcher Reset;
	
private:
    Ui::WidgetsDateTimeWidget *ui;
};

#endif // WIDGETSDATETIMEWIDGET_H
