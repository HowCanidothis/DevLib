#ifndef WIDGETSINPUTDIALOG_H
#define WIDGETSINPUTDIALOG_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

#ifdef UNITS_MODULE_LIB
#include <UnitsModule/internal.hpp>
    using WidgetsInputDialogConnectors = MeasurementWidgetConnectors;
#else
    using WidgetsInputDialogConnectors = LocalPropertiesWidgetConnectorsContainer;
#endif

namespace Ui {
class WidgetsInputDialog;
}

class WidgetsInputDialog : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
public:
    explicit WidgetsInputDialog(QWidget *parent = nullptr);
    ~WidgetsInputDialog();

    LocalPropertyString Content;
#ifdef UNITS_MODULE_LIB
    void AddMeasurement(const QString& label, const Measurement* measurement, LocalPropertyDouble* property);
#endif
    void AddDouble(const QString& label, LocalPropertyDouble* property);
    void AddLineText(const QString& label, LocalPropertyString* property);

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
    Ui::WidgetsInputDialog *ui;
    WidgetsInputDialogConnectors m_connectors;
    QVector<FAction> m_resets;

};

#endif // WIDGETSINPUTDIALOG_H
