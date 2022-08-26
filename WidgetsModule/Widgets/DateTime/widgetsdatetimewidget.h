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
    explicit WidgetsDateTimeWidget(QWidget *parent = nullptr);
    ~WidgetsDateTimeWidget() override;
	
    void ConnectModel(LocalPropertyDateTime* modelProperty, bool reactive = true);

    LocalPropertyBool NowEnabled;
    LocalPropertyDateTime CurrentDateTime;
    LocalPropertyLocale Locale;

	Dispatcher OnNowActivate;
	Dispatcher OnApplyActivate;
	
private:
    Ui::WidgetsDateTimeWidget *ui;
	DispatcherConnectionsSafe m_connections;
};

#endif // WIDGETSDATETIMEWIDGET_H
