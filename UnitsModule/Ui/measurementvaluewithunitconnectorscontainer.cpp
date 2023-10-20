#include "measurementvaluewithunitconnectorscontainer.h"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#ifdef WIDGETS_MODULE_LIB
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

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, bool reactive)
{
    AddConnector(measurement, &property->DisplayValue, spinBox, reactive);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, &property->DisplayValue, spinBox, label, translationHandler, labelUpdaters, reactive);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, &property->DisplayValue, spinBox, label, translationHandler, labelUpdaters, reactive);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleDisplay* property, QDoubleSpinBox* spinBox, QCheckBox* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, property, spinBox, reactive);
    AddTranslationConnector<LocalPropertiesCheckBoxConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, bool reactive)
{
    AddConnector(measurement, &property->Value, spinBox, reactive);
    spinBox->MakeOptional(&property->IsValid).MakeSafe(m_connections);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, property, spinBox, reactive);
    AddTranslationConnector<LocalPropertiesLabelConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, property, spinBox, reactive);
    AddTranslationConnector<LocalPropertiesLineEditConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDoubleOptional* property, WidgetsDoubleSpinBoxWithCustomDisplay* spinBox, QCheckBox* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, property, spinBox, reactive);
    AddTranslationConnector<LocalPropertiesCheckBoxConnector>(measurement, label, translationHandler, labelUpdaters);
}

#endif

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, bool reactive)
{
    auto data = createPropertyData(measurement, property);
    auto* measurementProperty = &data->Property;

    auto updatePrecision = [measurementProperty, spinBox]{
        QSignalBlocker blocker(spinBox);
        spinBox->setDecimals(measurementProperty->Precision);
        spinBox->setValue(measurementProperty->Value);
    };

    data->Property.Precision.OnChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, updatePrecision).MakeSafe(m_connections);
    AddConnector<LocalPropertiesDoubleSpinBoxConnector>(&data->Property.Value, spinBox, nullptr, reactive);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, QLabel* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, property, spinBox, reactive);
    AddTranslationConnector<LocalPropertiesLabelConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, QLineEdit* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, property, spinBox, reactive);
    AddTranslationConnector<LocalPropertiesLineEditConnector>(measurement, label, translationHandler, labelUpdaters);
}

void MeasurementWidgetConnectors::AddConnector(const Measurement* measurement, LocalPropertyDouble* property, QDoubleSpinBox* spinBox, QCheckBox* label, const FTranslationHandler& translationHandler, const QVector<Dispatcher*>& labelUpdaters, bool reactive)
{
    AddConnector(measurement, property, spinBox, reactive);
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
