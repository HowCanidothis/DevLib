#ifndef WIDGETSTIMEWIDGET_H
#define WIDGETSTIMEWIDGET_H

#include <QTime>
#include <QWidget>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsTimeWidget;
}

enum class DayType {
    AM,
    PM,
    First = AM,
    Last = PM,
};

template<>
struct EnumHelper<DayType>
{
    static QStringList GetNames() { return { tr("AM"), tr("PM")}; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

class WidgetsTimeWidget : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
public:
    explicit WidgetsTimeWidget(QWidget *parent = nullptr);
    ~WidgetsTimeWidget();

    LocalPropertyTime CurrentTime;

private:
    LocalPropertyInt Hour;
    LocalPropertyInt Minutes;
	LocalPropertySequentialEnum<DayType> Type;

    Ui::WidgetsTimeWidget *ui;
    DispatcherConnectionsSafe m_connections;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
};

#endif // WIDGETSTIMEWIDGET_H
