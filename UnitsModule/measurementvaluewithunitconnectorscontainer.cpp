#include "measurementvaluewithunitconnectorscontainer.h"

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox)
{
    auto data = createPropertyData(measurement, property);
    AddConnector<LocalPropertiesDoubleSpinBoxConnector>(&data->Property.Value, spinBox);

    auto* measurementProperty = &data->Property;

    auto updatePrecision = [data, measurementProperty, spinBox]{
        spinBox->setDecimals(measurementProperty->Precision);
    };

    data->Property.Precision.OnChange.Connect(this, updatePrecision).MakeSafe(m_connections);
    updatePrecision();
}

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLabelConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLineEditConnector>(measurement, label, translationHandler, labelUpdaters);
}

SharedPointer<MeasurementValueWithUnitConnectorsContainer::PropertyData> MeasurementValueWithUnitConnectorsContainer::createPropertyData(const Name& measurement, LocalPropertyDouble* property)
{
    auto data = ::make_shared<PropertyData>(measurement);
    data->Property.Connect(property);
    m_properties.append(data);
    return data;
}

SharedPointer<MeasurementValueWithUnitConnectorsContainer::TranslationData> MeasurementValueWithUnitConnectorsContainer::createTranslationData(const Name& measurement, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    auto data = ::make_shared<TranslationData>(measurement, translationHandler);
    for(auto* updater : labelUpdaters) {
        data->Translation.Retranslate.ConnectFrom(*updater).MakeSafe(m_connections);
    }
    m_properties.append(data);
    return data;
}

MeasurementValueWithUnitConnectorsContainer::TranslationData::TranslationData(const Name& measurement, const FTranslationHandler& translationHandler)
    : Translation(translationHandler)
{
    MeasurementTranslatedString::AttachToTranslatedString(Translation, translationHandler, { measurement });
}

MeasurementValueWithUnitConnectorsContainer::PropertyData::PropertyData(const Name& measurement)
    : Property(measurement)
{}
