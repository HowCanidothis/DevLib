#ifndef MEASUREMENTVALUEWITHUNITCONNECTORSCONTAINER_H
#define MEASUREMENTVALUEWITHUNITCONNECTORSCONTAINER_H

#include "UnitsModule/measurementunitmanager.h"

#include <WidgetsModule/internal.hpp>

class MeasurementDoubleSpinBoxWrapper
{
public:
    MeasurementDoubleSpinBoxWrapper(const Measurement* measurementType, QDoubleSpinBox* spinBox);

    void SetRange(double min, double max);
    void SetMinimum(double min, double offset = 0.0);
    void SetMaximum(double max, double offset = 0.0);
protected:
    QDoubleSpinBox* m_spinBox;
    MeasurementUnit::FTransform m_baseToUnitConverter;
};

class MeasurementWidgetConnectors : public LocalPropertiesWidgetConnectorsContainer
{
    using Super = LocalPropertiesWidgetConnectorsContainer;
    using FTranslationHandler = std::function<QString ()>;

    struct BaseData
    {
        virtual ~BaseData(){}
    };

    struct PropertyData : BaseData
    {
       MeasurementProperty Property;
       PropertyData(const Measurement* measurement);
    };

    struct TranslationData : BaseData
    {
        using Super = BaseData;
        TranslatedString Translation;

        TranslationData(const Measurement* measurement, const FTranslationHandler& translationHandler);
    };

public:
    template<class T, typename ... Args>
    T* AddConnector(Args... args)
    {
        return Super::AddConnector<T>(args...);
    }

    template<class ConnectorType, class T>
    void AddTranslationConnector(const Measurement* measurement, T* label, const MeasurementWidgetConnectors::FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {})
    {
        if(translationHandler == nullptr) {
            if(label->text().isEmpty()) {
                AddConnector<ConnectorType>(&const_cast<Measurement*>(measurement)->CurrentUnitLabel, label);
            } else {
                auto text = label->text();
                auto data = createTranslationData(measurement, TR(label->text().contains(MEASUREMENT_UN) ? label->text() : text,label, text), labelUpdaters);
                AddConnector<ConnectorType>(&data->Translation, label);
            }
        } else {
            auto data = createTranslationData(measurement, translationHandler, labelUpdaters);
            AddConnector<ConnectorType>(&data->Translation, label);
        }
    }

#ifdef WIDGETS_MODULE_LIB
    void AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, class QDoubleSpinBox* spinBox);
    void AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, class QLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    void AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, class QLineEdit* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    void AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, class QCheckBox* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});

    void AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, class WidgetsDoubleSpinBoxWithCustomDisplay* spinBox);
    void AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, class QLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    void AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, class QLineEdit* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    void AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, class QCheckBox* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
#endif

    void AddConnector(const Measurement* measurement, LocalPropertyDouble* property, class QDoubleSpinBox* spinBox);
    void AddConnector(const Measurement* measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, class QLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    void AddConnector(const Measurement* measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, class QLineEdit* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    void AddConnector(const Measurement* measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, class QCheckBox* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});


protected:
    void onClear()
    {
        m_properties.clear();
        m_connections.clear();
    }

private:
    SharedPointer<PropertyData> createPropertyData(const Measurement* measurement, LocalPropertyDouble* property);
    SharedPointer<TranslationData> createTranslationData(const Measurement* measurement, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters);

private:
    QVector<SharedPointer<BaseData>> m_properties;
    DispatcherConnectionsSafe m_connections;
};
#endif // MEASUREMENTVALUEWITHUNITCONNECTORSCONTAINER_H
