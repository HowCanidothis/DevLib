#include "measurementvaluewithunitconnectorscontainer.h"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include <WidgetsModule/internal.hpp>

MeasurementDoubleSpinBoxWrapper::MeasurementDoubleSpinBoxWrapper(const Measurement* measurement, QDoubleSpinBox* spinBox)
    : m_spinBox(spinBox)
{
    m_baseToUnitConverter = measurement->GetCurrentUnit()->GetBaseToUnitConverter();
    spinBox->setDecimals(measurement->CurrentPrecision);
    spinBox->setSingleStep(measurement->CurrentStep);
}

void MeasurementDoubleSpinBoxWrapper::SetRange(double min, double max)
{
    m_spinBox->setRange(m_baseToUnitConverter(min),m_baseToUnitConverter(max));
}

void MeasurementDoubleSpinBoxWrapper::SetMinimum(double min, double offset)
{
    m_spinBox->setMinimum(m_baseToUnitConverter(min) + offset);
}

void MeasurementDoubleSpinBoxWrapper::SetMaximum(double max, double offset)
{
    m_spinBox->setMaximum(m_baseToUnitConverter(max) + offset);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxLayout* spinBox, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox->spinBox(), spinBox->lineEdit(), translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, WidgetsDoubleSpinBoxLayout* spinBox, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox->spinBox(), spinBox->label(), translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox)
{
    AddConnector(measurement, &property->DisplayValue, spinBox);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, &property->DisplayValue, spinBox, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, &property->DisplayValue, spinBox, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QCheckBox* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesCheckBoxConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox)
{
    AddConnector(measurement, &property->Value, spinBox);
    spinBox->MakeOptional(&property->IsValid).MakeSafe(m_connections);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLabelConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLineEditConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QCheckBox* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesCheckBoxConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, ElidedLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLabelConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, ElidedLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLabelConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox)
{
    auto data = createPropertyData(measurement, property);
    auto* measurementProperty = &data->Property;

    auto updatePrecision = [measurementProperty, spinBox]{
        QSignalBlocker blocker(spinBox.SpinBox);
        spinBox->setDecimals(measurementProperty->Precision);
        spinBox->setValue(measurementProperty->Value);
    };

    data->Property.Precision.OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, updatePrecision).MakeSafe(m_connections);
    AddConnector<LocalPropertiesDoubleSpinBoxConnector>(&data->Property.Value, spinBox, nullptr);
}


void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLabelConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesLineEditConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, const LocalPropertiesDoubleSpinBoxConnectorExtractor& spinBox, QCheckBox* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    AddConnector(measurement, property, spinBox);
    AddTranslationConnector<LocalPropertiesCheckBoxConnector>(measurement, label, translationHandler, labelUpdaters);
}

SharedPointer<MeasurementWidgetConnectors::PropertyData> MeasurementWidgetConnectors::createPropertyData(const Measurement* measurement, LocalPropertyDouble* property)
{
    auto data = ::make_shared<PropertyData>(measurement);
    data->Property.Connect(property);
    m_properties.append(data);
    return data;
}

SharedPointer<MeasurementWidgetConnectors::TranslationData> MeasurementWidgetConnectors::createTranslationData(const Measurement* measurement, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters)
{
    auto data = ::make_shared<TranslationData>(measurement, translationHandler);
    for(auto* updater : labelUpdaters) {
        data->Translation.Retranslate.ConnectFrom(CONNECTION_DEBUG_LOCATION, *updater).MakeSafe(m_connections);
    }
    m_properties.append(data);
    return data;
}

MeasurementWidgetConnectors::TranslationData::TranslationData(const Measurement* measurement, const FTranslationHandler& translationHandler)
    : Translation(translationHandler)
{
    MeasurementTranslatedString::AttachToTranslatedString(Translation, translationHandler, { measurement });
}

MeasurementWidgetConnectors::PropertyData::PropertyData(const Measurement* measurement)
    : Property(measurement)
{}
