#ifndef MEASUREMENTVALUEWITHUNITCONNECTORSCONTAINER_H
#define MEASUREMENTVALUEWITHUNITCONNECTORSCONTAINER_H

#include "measurementunitmanager.h"

class MeasurementValueWithUnitConnectorsContainer : public LocalPropertiesWidgetConnectorsContainer
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
       PropertyData(const Name& measurement);
    };

    struct TranslationData : BaseData
    {
        using Super = BaseData;
        TranslatedString Translation;

        TranslationData(const Name& measurement, const FTranslationHandler& translationHandler);
    };

public:
    template<class T, typename ... Args>
    T* AddConnector(Args... args)
    {
        return Super::AddConnector<T>(args...);
    }

    template<class ConnectorType, class T>
    void AddTranslationConnector(const Name& measurement, T* label, const MeasurementValueWithUnitConnectorsContainer::FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {})
    {
        if(translationHandler == nullptr) {
            AddConnector<ConnectorType>(&MeasurementManager::GetInstance().GetMeasurement(measurement)->CurrentUnitLabel, label);
        } else {
            auto data = createTranslationData(measurement, translationHandler, labelUpdaters);
            AddConnector<ConnectorType>(&data->Translation, label);
        }
    }

    void AddConnector(const Name& measurement, LocalPropertyDouble* property, class QDoubleSpinBox* spinBox);
    void AddConnector(const Name& measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, class QLabel* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});
    void AddConnector(const Name& measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, class QLineEdit* label, const FTranslationHandler& translationHandler = nullptr, const QVector<Dispatcher*>& labelUpdaters = {});

protected:
    void onClear()
    {
        m_properties.clear();
        m_connections.clear();
    }

private:
    SharedPointer<PropertyData> createPropertyData(const Name& measurement, LocalPropertyDouble* property);
    SharedPointer<TranslationData> createTranslationData(const Name& measurement, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters);

private:
    QVector<SharedPointer<BaseData>> m_properties;
    DispatcherConnectionsSafe m_connections;
};
#endif // MEASUREMENTVALUEWITHUNITCONNECTORSCONTAINER_H
