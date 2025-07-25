#ifndef WIDGETSINPUTDIALOGVIEW_H
#define WIDGETSINPUTDIALOGVIEW_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
    using WidgetsInputDialogConnectors = MeasurementWidgetConnectors;
#else
    using WidgetsInputDialogConnectors = LocalPropertiesWidgetConnectorsContainer;
#endif

namespace Ui {
class WidgetsInputDialogView;
}

class WidgetsInputDialogView : public QWidget
{
    Q_OBJECT
    using Super = QWidget;
public:
    explicit WidgetsInputDialogView(QWidget *parent = nullptr);
    ~WidgetsInputDialogView();

    LocalPropertyString Content;
#ifdef UNITS_MODULE_LIB
    void AddMeasurement(const QString& label, const Measurement* measurement, LocalPropertyDouble* property);
#endif
    void AddInt(const QString& label, LocalPropertyInt* property);
    void AddDouble(const QString& label, LocalPropertyDouble* property);
    void AddLineText(const QString& label, LocalPropertyString* property, const QStringList& keys = QStringList(), bool encrypted = false);
    void AddMonth(const QString& label, LocalPropertyDate* property);
    void AddDate(const QString& label, LocalPropertyDate* property);
    void AddDateTime(const QString& label, LocalPropertyDateTime* property);
    void AddDateRange(const QString& label, LocalPropertyDate* from, LocalPropertyDate* to);
    void AddDateTimeRange(const QString& label, LocalPropertyDateTime* from, LocalPropertyDateTime* to);
    void Reset();

private:
    template<class T>
    void saveProperty(T* property)
    {
        auto value = property->Native();
        m_resets.append([property, value]{
            *property = value;
        });
    }

private:
    Ui::WidgetsInputDialogView *ui;
    WidgetsInputDialogConnectors m_connectors;
    QVector<FAction> m_resets;
    DispatcherConnectionsSafe m_connections;

};

#endif // WIDGETSINPUTDIALOGVIEW_H
