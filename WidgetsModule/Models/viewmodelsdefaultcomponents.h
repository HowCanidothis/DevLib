#ifndef VIEWMODELSDEFAULTCOMPONENTS_H
#define VIEWMODELSDEFAULTCOMPONENTS_H

#include "modelstablebase.h"

template<class T> class ModelsErrorComponent;

class TViewModelsColumnComponentsBuilderBase
{
    using Super = QObject;
public:
    TViewModelsColumnComponentsBuilderBase(ViewModelsTableBase* model)
        : m_viewModel(model)
    {}

    TViewModelsColumnComponentsBuilderBase& AddDefaultColors(LocalPropertyColor* enabledCellColor, LocalPropertyColor* disabledCellColor,
                                   LocalPropertyColor* enabledTextColor, LocalPropertyColor* disabledTextColor);

protected:
    ViewModelsTableBase* m_viewModel;
};

template<class T>
struct ReferenceHelper
{
    using reference = T&;
    using const_reference = const T&;
};

template<class T>
struct ReferenceHelper<T*>
{
    using reference = T*;
    using const_reference = const T*;
};

template<class Wrapper, typename ValueType = typename ReferenceHelper<typename Wrapper::value_type>::reference, typename ConstValueType = typename ReferenceHelper<typename Wrapper::value_type>::const_reference>
class TViewModelsColumnComponentsBuilder : TViewModelsColumnComponentsBuilderBase
{
    using Super = TViewModelsColumnComponentsBuilderBase;

public:
    using FModelGetter = std::function<QVariant (ConstValueType)>;
    using FModelSetter = std::function<FAction (const QVariant& data, ValueType)>;

    TViewModelsColumnComponentsBuilder(ViewModelsTableBase* viewModel, const std::function<Wrapper* ()>& modelGetter)
        : Super(viewModel)
        , m_modelGetter(modelGetter)
        , m_currentColumn(-1)
#ifdef UNITS_MODULE_LIB
        , m_currentMeasurement(nullptr)
#endif
    {}
    template<class T>
    TViewModelsColumnComponentsBuilder(T* viewModel)
        : TViewModelsColumnComponentsBuilder(viewModel, [viewModel]{ return viewModel->GetData().get(); })
    {}

    ~TViewModelsColumnComponentsBuilder()
    {
#ifdef UNITS_MODULE_LIB
        AttachDependencies();
#endif
    }

    TViewModelsColumnComponentsBuilder& AddDefaultColors()
    {
        auto& settings = SharedSettings::GetInstance();
        Super::AddDefaultColors(&settings.StyleSettings.EnabledTableCellColor, &settings.StyleSettings.DisabledTableCellColor,
                         &settings.StyleSettings.EnabledTableCellTextColor, &settings.StyleSettings.DisabledTableCellTextColor);
        return *this;
    }

    template<typename T2>
    TViewModelsColumnComponentsBuilder& AddErrorComponent(ModelsErrorComponent<Wrapper>* component, const std::map<qint32, QVector<qint64>>& columns, const std::function<const T2&(ConstValueType&)>& extractor = [](ConstValueType& d)->ConstValueType&{return d;})
    {
        auto* viewModel = m_viewModel;
        auto modelGetter = m_modelGetter;
        for(const auto& [column, errorsStack] : columns) {
            auto errorsStackCopy = errorsStack;
            viewModel->ColumnComponents.AddComponent(Qt::DecorationRole, column, ViewModelsTableColumnComponents::ColumnComponentData()
                                                   .SetGetter([viewModel, component, errorsStackCopy, modelGetter, extractor](const QModelIndex& index) {
                                                        ConstValueType data = modelGetter()->At(index.row());
                                                        return component->ErrorIcon(extractor(data), errorsStackCopy, viewModel->GetIconsContext());
                                                    }));
            viewModel->ColumnComponents.AddComponent(Qt::ToolTipRole, column, ViewModelsTableColumnComponents::ColumnComponentData()
                                                   .SetGetter([component, errorsStackCopy, modelGetter, extractor](const QModelIndex& index) {
                                                        ConstValueType data = modelGetter()->At(index.row());
                                                        return component->ErrorString(extractor(data), errorsStackCopy);
                                                    }));
        }
        return *this;
    }

    TViewModelsColumnComponentsBuilder& AddColumn(qint32 column, const FTranslationHandler& header, const FModelGetter& getterUi, const FModelSetter& setter = nullptr, const FModelGetter& inGetter = nullptr)
    {
        m_currentColumn = column;
        auto modelGetter = m_modelGetter;
        ViewModelsTableColumnComponents::ColumnComponentData displayRoleComponent;
        auto getter = inGetter;
        if(getter == nullptr) {
            getter = getterUi;
        }

        displayRoleComponent.GetterHandler = [modelGetter, getterUi](const QModelIndex& index) -> std::optional<QVariant> {
            const auto& viewModel = modelGetter();
            if(viewModel == nullptr) {
                return "-";
            }
            if(index.row() >= viewModel->GetSize()) {
                return "-";
            }
            return getterUi(viewModel->At(index.row()));
        };
        displayRoleComponent.GetHeaderHandler = [header]{ return header(); };

        m_viewModel->ColumnComponents.AddComponent(Qt::DisplayRole, column, displayRoleComponent);

        auto editRoleGetter = [modelGetter, getter](const QModelIndex& index) -> std::optional<QVariant> {
            const auto& viewModel = modelGetter();
            if(viewModel == nullptr) {
                return QVariant();
            }
            if(index.row() >= viewModel->GetSize()) {
                return QVariant();
            }
            return getter(viewModel->At(index.row()));
        };

        ViewModelsTableColumnComponents::ColumnComponentData editRoleComponent;
        editRoleComponent.GetterHandler = editRoleGetter;

        if(setter != nullptr) {
            editRoleComponent.SetterHandler = [modelGetter, setter, column](const QModelIndex& index, const QVariant& data) -> std::optional<bool> {
                const auto& viewModel = modelGetter();
                if(viewModel == nullptr) {
                    return false;
                }
                if(index.row() >= viewModel->GetSize()) {
                    return false;
                }
                QVariant toSet;
                if(data.toString() != "-") {
                    toSet = data;
                }
                return viewModel->EditWithCheck(index.row(), [&](ValueType value){ return setter(toSet, value); }, {column});
            };

            m_viewModel->ColumnComponents.AddFlagsComponent(column, [](qint32) { return ViewModelsTableBase::StandardEditableFlags(); });
        } else {
            m_viewModel->ColumnComponents.AddFlagsComponent(column, [](qint32) { return ViewModelsTableBase::StandardNonEditableFlags(); });
        }

        m_viewModel->ColumnComponents.AddComponent(Qt::EditRole, column, editRoleComponent);

        return *this;
    }

    TViewModelsColumnComponentsBuilder& AddStringColumn(qint32 column, const FTranslationHandler& header, const std::function<QString& (ValueType)>& getter) {
        return AddColumn<QString>(column, header, getter);
    }

    template<typename T>
    TViewModelsColumnComponentsBuilder& AddColumn(qint32 column, const FTranslationHandler& header, const std::function<T& (ValueType)>& getter) {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType& data = const_cast<ValueType>(constData);
            return getter(data);
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{ getter(data) = value.value<T>();};
        });
    }

    template<typename T>
    TViewModelsColumnComponentsBuilder& AddReadOnlyColumn(qint32 column, const FTranslationHandler& header, const std::function<T(ConstValueType)>& getter) {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            return QVariant::fromValue(getter(constData));
        });
    }

    template<class Enum>
    TViewModelsColumnComponentsBuilder& AddEnumColumn(qint32 column, const FTranslationHandler& header, const std::function<Enum& (ValueType)>& getter, bool readOnly = false)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TranslatorManager::ToVariant<Enum>(getter(data));
        }, readOnly ? FModelSetter() : [getter](const QVariant& value, ValueType data) -> FAction {
            return TranslatorManager::SetterFromString(getter(data), value.toString());
        });
    }

    template<class Enum>
    TViewModelsColumnComponentsBuilder& AddEnumPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertySequentialEnum<Enum>& (ValueType)>& getter, bool readOnly = false)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TranslatorManager::ToVariant<Enum>(getter(data));
        }, readOnly ? FModelSetter() : [getter](const QVariant& value, ValueType data) -> FAction {
            return getter(data).SetterFromString(value.toString());
        });
    }

    template<typename T>
    TViewModelsColumnComponentsBuilder& AddPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalProperty<T>& (ValueType)>& getter, bool readOnly = false)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return getter(data).Native();
        }, readOnly ? FModelSetter() : [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{ getter(data) = value.value<T>();};
        });
    }
    TViewModelsColumnComponentsBuilder& AddIdPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalProperty<Name>& (ValueType)>& getter, bool readOnly = false){
        return AddPropertyColumn<Name>(column, header, getter, readOnly);
    }
    TViewModelsColumnComponentsBuilder& AddIntPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyInt& (ValueType)>& getter, bool readOnly = false){
        return AddPropertyColumn<int>(column, header, getter, readOnly);
    }
    TViewModelsColumnComponentsBuilder& AddDoublePropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDouble& (ValueType)>& getter, bool readOnly = false){
        return AddPropertyColumn<double>(column, header, getter, readOnly);
    }
    TViewModelsColumnComponentsBuilder& AddStringPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyString& (ValueType)>& getter, bool readOnly = false){
        return AddPropertyColumn<QString>(column, header, getter, readOnly);
    }

    struct TimeParams
    {
        using FGetter = std::function<LocalPropertyDateTime& (ValueType)>;

        TimeParams(const FGetter& getter)
            : TimeShift(nullptr)
            , IsReadOnly(false)
            , Getter(getter)
        {}

        TimeParams& SetTimeShift(const double* timeShift)
        {
            TimeShift = timeShift;
            return *this;
        }
        TimeParams& SetReadOnly(bool readOnly)
        {
            IsReadOnly = readOnly;
            return *this;
        }

        const double* TimeShift;
        bool IsReadOnly;
        FGetter Getter;
    };

    TimeParams CreateTimeParams(const typename TimeParams::FGetter& getter) { return TimeParams(getter); }

    TViewModelsColumnComponentsBuilder& AddTimePropertyColumn(qint32 column, const FTranslationHandler& header, const TimeParams& params){
        return AddColumn(column, header, [params](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(params.TimeShift != nullptr) {
                return TimeToString(params.Getter(data).Native().toOffsetFromUtc(*params.TimeShift).time());
            }
            return TimeToString(params.Getter(data).Native().time());
        }, params.IsReadOnly ? FModelSetter() : [params](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                auto& property = params.Getter(data);
                if(params.TimeShift != nullptr) {
                    property = QDateTime(property.Native().date(), TimeFromVariant(value), Qt::OffsetFromUTC, *params.TimeShift);
                    return;
                }
                property = QDateTime(property.Native().date(), TimeFromVariant(value));
            };
        }, [params](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(params.TimeShift != nullptr) {
                return params.Getter(data).Native().toOffsetFromUtc(*params.TimeShift).time();
            }
            return params.Getter(data).Native().time();
        });
    }
    TViewModelsColumnComponentsBuilder& AddDateTimePropertyColumn(qint32 column, const FTranslationHandler& header, const TimeParams& params){
        return AddColumn(column, header, [params](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(params.TimeShift != nullptr) {
                auto dateTime = params.Getter(data).Native();
                return DateTimeToString(dateTime.toOffsetFromUtc(*params.TimeShift));
            }
            return DateTimeToString(params.Getter(data).Native());
        }, params.IsReadOnly ? FModelSetter() : [params](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                if(params.TimeShift != nullptr) {
                    auto& property = params.Getter(data);
                    auto inputDateTime = DateTimeFromVariant(value);
                    property = QDateTime(inputDateTime.date(), inputDateTime.time(), Qt::OffsetFromUTC, *params.TimeShift);
                    return;
                }
                params.Getter(data) = DateTimeFromVariant(value);
        };
        }, [params](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(params.TimeShift != nullptr) {
                auto dateTime = params.Getter(data).Native();
                return dateTime.toOffsetFromUtc(*params.TimeShift);
            }
            return params.Getter(data).Native();
        });
    }

#ifdef UNITS_MODULE_LIB
    TViewModelsColumnComponentsBuilder& SetCurrentMeasurement(const Measurement* measurement)
    {
        if(m_currentMeasurement != measurement){
            AttachDependencies();
            m_currentMeasurement = measurement;
        }
        return *this;
    }

    void AttachDependencies()
    {
        if(!m_currentMeasurementColumns.isEmpty()) {
            m_viewModel->AttachDependence(CONNECTION_DEBUG_LOCATION, &m_currentMeasurement->OnChanged, m_currentMeasurementColumns.First(), m_currentMeasurementColumns.Last());
            m_currentMeasurementColumns.clear();
        }
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumnCalculable(qint32 column, const FTranslationHandler& header, const std::function<double (ConstValueType)>& getter)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return setMeasurmentUnit(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto concreteValue = getter(const_cast<ValueType>(value));
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }
            return pMeasurement->FromBaseToUnitUi(concreteValue);
        }, FModelSetter(), [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(value)));
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumnCalculableOpt(qint32 column, const FTranslationHandler& header, const std::function<std::optional<double>(ConstValueType)>& getter)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return setMeasurmentUnit(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto dataValue = getter(const_cast<ValueType>(value));
            if(!dataValue.has_value()) {
                return "-";
            }
            const auto& concreteValue = dataValue.value();
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }

            return pMeasurement->FromBaseToUnitUi(concreteValue);
        }, FModelSetter(), [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto dataValue = getter(const_cast<ValueType>(value));
            if(!dataValue.has_value()) {
                return QVariant();
            }
            return pMeasurement->FromBaseToUnit(dataValue.value());
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<double& (ValueType)>& getter, bool readOnly = false)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return setMeasurmentUnit(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto concreteValue = getter(const_cast<ValueType>(value));
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }
            return pMeasurement->FromBaseToUnitUi(concreteValue);
        }, readOnly ? FModelSetter() : [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{
                auto toSet = pMeasurement->FromUnitToBase(data.toDouble());
                auto& old = getter(value);
                if(!fuzzyCompare(old, toSet, pMeasurement->FromUnitToBase(epsilon(pMeasurement->CurrentPrecision) * 0.9))) {
                    old = toSet;
                }
            };
        }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(value)));
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumnLimits(const std::function<double(ConstValueType)>& min = [](ConstValueType){ return std::numeric_limits<double>().lowest(); }, const std::function<double(ConstValueType)>& max = [](ConstValueType){ return (std::numeric_limits<double>::max)(); })
    {
        qint32 column = m_currentColumn;
        auto modelGetter = m_modelGetter;
        Q_ASSERT(m_currentMeasurement != nullptr);
        auto pMeasurement = m_currentMeasurement;
        m_viewModel->ColumnComponents.AddComponent(UnitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetHeader([pMeasurement]{ return QVariant::fromValue(pMeasurement); }));
        m_viewModel->ColumnComponents.AddComponent(MinLimitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetGetter([min, pMeasurement, modelGetter](const QModelIndex& index) -> std::optional<QVariant> {
            const auto& viewModel = modelGetter();
            if(viewModel == nullptr || index.row() >= viewModel->GetSize()) {
                return std::nullopt;
            }
            return pMeasurement->FromBaseToUnit(min(viewModel->At(index.row())));
        }));

        m_viewModel->ColumnComponents.AddComponent(MaxLimitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetGetter([max, pMeasurement, modelGetter](const QModelIndex& index) -> std::optional<QVariant> {
            const auto& viewModel = modelGetter();
            if(viewModel == nullptr || index.row() >= viewModel->GetSize()) {
                return std::nullopt;
            }
            return pMeasurement->FromBaseToUnit(max(viewModel->At(index.row())));
        }));
        return *this;
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDouble& (ValueType)>& getter, bool readOnly = false)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        m_currentColumn = column;
        if(!readOnly) {
            addMeasurementLimits(getter);
        }
        return AddColumn(column, [header, pMeasurement]{ return setMeasurmentUnit(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto concreteValue = getter(const_cast<ValueType>(value)).Native();
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }
            return pMeasurement->FromBaseToUnitUi(concreteValue);
        }, readOnly ? FModelSetter() : [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{
                auto toSet = pMeasurement->FromUnitToBase(data.toDouble());
                auto& old = getter(value);
                if(!fuzzyCompare(old, toSet, pMeasurement->FromUnitToBase(epsilon(pMeasurement->CurrentPrecision) * 0.9))) {
                    old = toSet;
                }
            };
        }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(value)));
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDoubleOptional& (ValueType)>& getter, bool readOnly = false)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        m_currentColumn = column;
        if(!readOnly) {
            addMeasurementLimits([getter](ValueType value) -> LocalPropertyDouble& { return getter(value).Value; });
        }
        return AddColumn(column, [header, pMeasurement]{ return setMeasurmentUnit(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            const auto& concreteValue = getter(const_cast<ValueType>(value));
            if(!concreteValue.IsValid || qIsNaN(concreteValue.Value) || qIsInf(concreteValue.Value)) {
                return "-";
            }
            return pMeasurement->FromBaseToUnitUi(concreteValue.Value);
        }, readOnly ? FModelSetter() : [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{
                bool isDouble; auto dval = data.toDouble(&isDouble);
                if(isDouble){
                    auto& property = getter(value);
                    auto toSet = pMeasurement->FromUnitToBase(dval);
                    if(!fuzzyCompare(property.Value.Native(), toSet, pMeasurement->FromUnitToBase(epsilon(pMeasurement->CurrentPrecision) * 0.9))) {
                        property.Value = toSet;
                    }
                    property.IsValid = true;
                } else {
                    getter(value).IsValid = false;
                }
            };
        }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            const auto& concreteValue = getter(const_cast<ValueType>(value));
            if(!concreteValue.IsValid){
                return QVariant();
            }
            return pMeasurement->FromBaseToUnit(concreteValue.Value);
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column,
                                                             const FTranslationHandler& header,
                                                             const std::function<std::optional<double>& (ValueType)>& getter)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return setMeasurmentUnit(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType constValue) -> QVariant {
            auto& value = const_cast<ValueType>(constValue);
            auto& dataValue = getter(value);
            if(!dataValue.has_value()) {
                return "-";
            }
            const auto& concreteValue = dataValue.value();
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }
            return pMeasurement->FromBaseToUnitUi(concreteValue);
        }, [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{
                bool isDouble; auto dval = data.toDouble(&isDouble);
                if(isDouble){
                    getter(value) = pMeasurement->FromUnitToBase(dval);
                } else {
                    getter(value) = std::nullopt;
                }
            };
        }, [getter, pMeasurement](ConstValueType constValue) -> QVariant {
            auto& value = const_cast<ValueType>(constValue);
            auto& dataValue = getter(value);
            if(!dataValue.has_value()) {
                return QVariant();
            }
            return pMeasurement->FromBaseToUnit(dataValue.value());
        });
    }

    static QString setMeasurmentUnit(const QString& string, const Measurement* measurment)
    {
        thread_local static QRegExp regExp(MEASUREMENT_UN);
        qint32 index = 0, stringIndex = 0;
        QString resultString;
        while((index = regExp.indexIn(string, index)) != -1) {
            resultString.append(QStringView(string.begin() + stringIndex, string.begin() + index).toString());
            resultString.append(measurment->CurrentUnitLabel);
            index += regExp.matchedLength();
            stringIndex = index;
        }
        resultString.append(QStringView(string.begin() + stringIndex, string.end()).toString());
        return resultString;
    }
private:
    TViewModelsColumnComponentsBuilder& addMeasurementLimits(const std::function<LocalPropertyDouble& (ValueType)>& getter)
    {
        qint32 column = m_currentColumn;
        auto modelGetter = m_modelGetter;
        Q_ASSERT(m_currentMeasurement != nullptr);
        auto pMeasurement = m_currentMeasurement;
        m_viewModel->ColumnComponents.AddComponent(UnitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetHeader([pMeasurement]{ return QVariant::fromValue(pMeasurement); }));
        m_viewModel->ColumnComponents.AddComponent(MinLimitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetGetter([getter, pMeasurement, modelGetter](const QModelIndex& index) -> QVariant {
            const auto& viewModel = modelGetter();
            if(viewModel == nullptr || index.row() >= viewModel->GetSize()) {
                return std::numeric_limits<double>().max();
            }
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(viewModel->At(index.row()))).GetMin());
        }));
        m_viewModel->ColumnComponents.AddComponent(MaxLimitRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetGetter([getter, pMeasurement, modelGetter](const QModelIndex& index) -> QVariant {
            const auto& viewModel = modelGetter();
            if(viewModel == nullptr || index.row() >= viewModel->GetSize()) {
                return std::numeric_limits<double>().max();
            }
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(viewModel->At(index.row()))).GetMax());
        }));
        return *this;
    }

#endif
private:
    std::function<Wrapper* ()> m_modelGetter;
    qint32 m_currentColumn;
#ifdef UNITS_MODULE_LIB
    const class Measurement* m_currentMeasurement;
    Array<qint32> m_currentMeasurementColumns;
#endif
};


#define ViewModelsBuilderColumnUiReadOnly(column, header, getter, getterUi) \
    builder.AddColumn(column, []{ return header; }, \
    [](const auto& data)->QVariant { return QVariant::fromValue(getterUi); }, \
    nullptr, \
    [](const auto& data)->QVariant { return QVariant::fromValue(getter); })

#define ViewModelsBuilderColumnReadOnly(column, header, getter) \
    builder.AddColumn(column, []{ return header; }, \
    [](const auto& data)->QVariant { return QVariant::fromValue(getter); })

#define ViewModelsBuilderColumnUi(column, header, getter, getterUi, setter) \
    builder.AddColumn(column, []{ return header; }, \
    [](const auto& data)->QVariant { return QVariant::fromValue(getterUi); }, \
    [](const QVariant& value, auto& data) -> FAction { return [&]{ getter = setter; }; }, \
    [](const auto& data)->QVariant { return QVariant::fromValue(getter); })

#define ViewModelsBuilderColumn(column, header, getter, setter) \
    builder.AddColumn(column, []{ return header; }, \
    [](const auto& data)->QVariant { return QVariant::fromValue(getter); }, \
    [](const QVariant& value, auto& data) -> FAction { return [&]{ getter = setter; }; }, \
    [](const auto& data)->QVariant { return QVariant::fromValue(getter); })

#endif // VIEWMODELSDEFAULTCOMPONENTS_H
