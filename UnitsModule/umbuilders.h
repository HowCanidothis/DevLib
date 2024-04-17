#ifndef UMBUILDERS_H
#define UMBUILDERS_H

#include <SharedModule/internal.hpp>

template<template<class T> typename Helper, class Context>
template<class T2>
ParseFactoryBuilder& ParseFactoryBuilder<Helper<T>, Context>::RegisterMeasurementOptional(const Name& key, const MeasurementUnit& unit,
                              const FPropertyExtractor<std::optional<double>,T2>& targetPropertyExtractor,
                              const FTargetExtractor<T2>& targetExtractor,
                              const typename Helper<std::optional<double>>::extractor_type& extractor = &Helper<std::optional<double>>::Extract)
{
    return RegisterOptional<double, T2>(key, targetPropertyExtractor, targetExtractor, [extractor, &unit](const typename Helper<void>::parse_type& ref) -> std::optional<double> {
        auto result = extractor(ref);
        if(result.has_value()) {
            return unit.FromUnitToBase(result.value());
        }
        return result;
    });
}

template<template<class T> typename Helper, class Context>
template<class T2>
ParseFactoryBuilder& ParseFactoryBuilder<Helper<T>, Context>::RegisterMeasurementPropertyOptional(const Name& key, const MeasurementUnit& unit,
                              const FPropertyExtractor<LocalPropertyDoubleOptional,T2>& targetPropertyExtractor,
                              const FTargetExtractor<T2>& targetExtractor,
                              const typename Helper<std::optional<double>>::extractor_type& extractor = &Helper<std::optional<double>>::Extract)
{
    return RegisterPropertyOptional<LocalPropertyDoubleOptional, T2>(key, targetPropertyExtractor, targetExtractor, [extractor, &unit](const typename Helper<void>::parse_type& ref) -> std::optional<double> {
        auto result = extractor(ref);
        if(result.has_value()) {
            return unit.FromUnitToBase(result.value());
        }
        return result;
    });
}

template<template<class T> typename Helper, class Context>
template<class T2>
ParseFactoryBuilder& ParseFactoryBuilder<Helper<T>, Context>::RegisterMeasurementProperty(const Name& key, const MeasurementUnit& unit,
                              const FPropertyExtractor<LocalPropertyDouble,T2>& targetPropertyExtractor,
                              const FTargetExtractor<T2>& targetExtractor,
                              const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract)
{
    return RegisterProperty<LocalPropertyDouble, T2>(key, targetPropertyExtractor, targetExtractor, [extractor, &unit](const typename Helper<void>::parse_type& ref) -> double {
        return unit.FromUnitToBase(extractor(ref));
    });
}

template<template<class T> typename Helper, class Context>
template<class T2>
ParseFactoryBuilder& ParseFactoryBuilder<Helper<T>, Context>::RegisterMeasurementField(const Name& key, const MeasurementUnit& unit,
                              const FPropertyExtractor<double,T2>& targetPropertyExtractor,
                              const FTargetExtractor<T2>& targetExtractor,
                              const typename Helper<double>::extractor_type& extractor = &Helper<double>::Extract)
{
    return Insert(key, [extractor, targetExtractor, targetPropertyExtractor, &unit](const typename Helper<void>::parse_type& in, Context& context){
        auto* target = targetExtractor(context);
        if(target == nullptr) {
            return;
        }
        targetPropertyExtractor(*target) = unit.FromUnitToBase(extractor(in));
    });
}

template<template<class T> typename Helper, class Context>
static QString ParseFactoryBuilder<Helper<T>, Context>::setMeasurmentUnit(const QString& string, const Measurement* measurment)
{
    thread_local static QRegularExpression regExp(MEASUREMENT_UN);
    qint32 stringIndex = 0;
    QString resultString;
    auto it = regExp.globalMatch(string);
    while(it.hasNext()) {
        auto n = it.next();
        resultString.append(QStringView(string.begin() + stringIndex, string.begin() + n.capturedStart()).toString());
        resultString.append(measurment->CurrentUnitLabel);
        stringIndex = n.capturedEnd();
    }
    resultString.append(QStringView(string.begin() + stringIndex, string.end()).toString());
    return resultString;
}

template<template<class T> typename Helper, class Context>
TViewModelsColumnComponentsBuilder& ParseFactoryBuilder<Helper<T>, Context>::addMeasurementLimits(const std::function<void (qint32, qint32, const ViewModelsTableColumnComponents::ColumnComponentData&)>& addDelegate,
                                                         const FDoubleGetterConst& min = [](ConstValueType){ return std::numeric_limits<double>().lowest(); },
                                                         const FDoubleGetterConst& max = [](ConstValueType){ return (std::numeric_limits<double>::max)(); })
{
    qint32 column = m_currentColumn;
    auto modelGetter = m_modelGetter;
    Q_ASSERT(m_currentMeasurement != nullptr);
    auto pMeasurement = m_currentMeasurement;
    addDelegate(UnitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetHeader([pMeasurement]{ return QVariant::fromValue(pMeasurement); }));
    addDelegate(MinLimitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetGetter([min, pMeasurement, modelGetter](const QModelIndex& index) -> QVariant {
        const auto& viewModel = modelGetter();
        if(viewModel == nullptr || index.row() >= viewModel->GetSize()) {
            return std::numeric_limits<double>().max();
        }
        return pMeasurement->FromBaseToUnit(min(viewModel->At(index.row())));
    }));
    addDelegate(MaxLimitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetGetter([max, pMeasurement, modelGetter](const QModelIndex& index) -> QVariant {
        const auto& viewModel = modelGetter();
        if(viewModel == nullptr || index.row() >= viewModel->GetSize()) {
            return std::numeric_limits<double>().max();
        }
        return pMeasurement->FromBaseToUnit(max(viewModel->At(index.row())));
    }));
    return *this;
}

#endif // UMBUILDERS_H
