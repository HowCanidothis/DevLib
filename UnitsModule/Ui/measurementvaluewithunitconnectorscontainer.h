#ifndef MEASUREMENTVALUEWITHUNITCONNECTORSCONTAINER_H
#define MEASUREMENTVALUEWITHUNITCONNECTORSCONTAINER_H

#include "UnitsModule/measurementunitmanager.h"

struct LocalPropertiesDoubleSpinBoxConnectorExtractor;

namespace MeasurementProperties {

DECLARE_GLOBAL_CHAR(STWU) // SourceTextWithUnits

}

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
    ConnectorType* AddTranslationConnector(const Measurement* measurement, T* label, const MeasurementWidgetConnectors::FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {})
    {
        if(translationHandler == nullptr) {
            if(label->text().isEmpty()) {
                return AddConnector<ConnectorType>(&const_cast<Measurement*>(measurement)->CurrentUnitLabel, label);
            }
            auto text = label->text();
            if(text.contains(MEASUREMENT_UN)) {
                label->setProperty(MeasurementProperties::STWU, text);
            } else {
                auto prop = label->property(MeasurementProperties::STWU);
                if(prop.isNull()) {
                    text.append(' ').append('(').append(MEASUREMENT_UN).append(')');
                    label->setProperty(MeasurementProperties::STWU, text);
                } else {
                    text = prop.toString();
                }
            }
            auto data = createTranslationData(measurement, TR(text, =), labelUpdaters);
            return AddConnector<ConnectorType>(&data->Translation, label);
        } else {
            auto data = createTranslationData(measurement, translationHandler, labelUpdaters);
            return AddConnector<ConnectorType>(&data->Translation, label);
        }
    }

    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox);
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QLineEdit* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QCheckBox* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});

    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, class WidgetsDoubleSpinBoxLayout* spinBox, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox);
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QLineEdit* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QCheckBox* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class ElidedLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});

    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDouble* property, WidgetsDoubleSpinBoxLayout* spinBox, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class ElidedLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QLineEdit* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, class QCheckBox* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});

    LocalPropertiesDoubleSpinBoxConnector* AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox);

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
