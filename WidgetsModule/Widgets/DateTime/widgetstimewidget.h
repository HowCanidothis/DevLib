#ifndef WIDGETSTIMEWIDGET_H
#define WIDGETSTIMEWIDGET_H

#include <QTime>
#include <QWidget>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/Connectors/localpropertieswidgetconnector.h"

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

private:
    LocalPropertySequentialEnum<DayType> Type;
    Ui::WidgetsTimeWidget *ui;

    DispatcherConnectionsSafe m_connections;
    DispatcherConnectionsSafe m_dayTypeConnections;
    DispatcherConnectionsSafe m_hourTypeConnections;
    LocalPropertiesWidgetConnectorsContainer m_connectors;
    ScopedPointer<class TimeConverter> m_timeConverter;

public:
    LocalPropertyTime& CurrentTime;
};

#endif // WIDGETSTIMEWIDGET_H
