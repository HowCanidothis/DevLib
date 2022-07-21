#include "measurementvaluewithunitconnectorscontainer.h"

#ifdef WIDGETS_MODULE_LIB
#include <WidgetsModule/internal.hpp>

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox)
{
    AddConnector(measurement, &property->DisplayValue, spinBox);
}

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, &property->DisplayValue, spinBox, label, translationHandler, labelUpdaters);
}

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, &property->DisplayValue, spinBox, label, translationHandler, labelUpdaters);
}

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox)
{
    AddConnector(measurement, &property->Value, spinBox);
    spinBox->MakeOptional(&property->IsValid).MakeSafe(m_connections);
}

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLabelConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLineEditConnector>(measurement, label, translationHandler, labelUpdaters);
}

#endif

void MeasurementValueWithUnitConnectorsContainer::AddConnector(const Name& measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox)
{
    auto data = createPropertyData(measurement, property);
    auto* measurementProperty = &data->Property;

    auto updatePrecision = [measurementProperty, spinBox]{
        QSignalBlocker blocker(spinBox);
        spinBox->setDecimals(measurementProperty->Precision);
        spinBox->setValue(std::numeric_limits<double>::lowest());
        spinBox->setValue(measurementProperty->Value);
    };

    data->Property.Precision.OnChanged.Connect(CONNECTION_DEBUG_LOCATION, updatePrecision).MakeSafe(m_connections);
    updatePrecision();
    AddConnector<LocalPropertiesDoubleSpinBoxConnector>(&data->Property.Value, spinBox);
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
        data->Translation.Retranslate.ConnectFrom(CONNECTION_DEBUG_LOCATION, *updater).MakeSafe(m_connections);
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
