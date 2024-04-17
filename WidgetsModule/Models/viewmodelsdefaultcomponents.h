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
    using FModelModelGetter = std::function<Wrapper* ()>;
    using FModelGetter = std::function<QVariant (ConstValueType)>;
    using FModelSetter = std::function<FAction (const QVariant& data, ValueType)>;
    using FExternalModelSetter = std::function<FAction (const QVariant& data, ValueType, ConstValueType)>;

    TViewModelsColumnComponentsBuilder(ViewModelsTableBase* viewModel, const FModelModelGetter& modelGetter)
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
                return DASH;
            }
            if(index.row() >= viewModel->GetSize()) {
                return DASH;
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
                if(data.toString() != DASH) {
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
    TViewModelsColumnComponentsBuilder& AddIdPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyName& (ValueType)>& getter, bool readOnly = false){
        return AddPropertyColumn<Name>(column, header, getter, readOnly);
    }
    TViewModelsColumnComponentsBuilder& AddIntPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyInt& (ValueType)>& getter, bool readOnly = false){
        return AddPropertyColumn<int>(column, header, getter, readOnly);
    }
    TViewModelsColumnComponentsBuilder& AddDoublePropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDouble& (ValueType)>& getter, bool readOnly = false){
        return AddPropertyColumn<double>(column, header, getter, readOnly);
    }
    TViewModelsColumnComponentsBuilder& AddDoublePropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDoubleOptional& (ValueType)>& getter, bool readOnly = false){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            LocalPropertyDoubleOptional& property = getter(data);
            return property.IsValid ? QVariant(property.Value.Native()) : QVariant("-");
        }, readOnly ? FModelSetter() : [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                LocalPropertyDoubleOptional& property = getter(data);
                if(value.isNull()){
                    property.IsValid = false;
                } else {
                    property.Value = value.toDouble();
                    property.IsValid = true;
                }
            };
        });
    }
    TViewModelsColumnComponentsBuilder& AddDatePropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDate& (ValueType)>& getter, bool readOnly = false){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            LocalPropertyDate& property = getter(data);
            return property.Native().isValid() ? DateToString(property.Native()) : QVariant("-");
        }, readOnly ? FModelSetter() : [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                LocalPropertyDate& property = getter(data);
                property = LanguageSettings::DateFromVariant(value);
            };
        });
    }
    TViewModelsColumnComponentsBuilder& AddStringPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyString& (ValueType)>& getter, bool readOnly = false){
        return AddPropertyColumn<QString>(column, header, getter, readOnly);
    }

    template<class T>
    struct DateTimeParams
    {
        using FGetter = std::function<T& (ValueType)>;

        DateTimeParams(const FGetter& getter)
            : TimeShift(nullptr)
            , IsReadOnly(false)
            , Getter(getter)
        {}

        DateTimeParams& SetTimeShift(const double* timeShift)
        {
            TimeShift = timeShift;
            return *this;
        }
        DateTimeParams& SetReadOnly(bool readOnly)
        {
            IsReadOnly = readOnly;
            return *this;
        }

        const double* TimeShift;
        bool IsReadOnly;
        FGetter Getter;
    };

    DateTimeParams<LocalPropertyDateTime> CreateDateTimePropertyParams(const typename DateTimeParams<LocalPropertyDateTime>::FGetter& getter) { return DateTimeParams<LocalPropertyDateTime>(getter); }
    DateTimeParams<QDateTime> CreateDateTimeParams(const typename DateTimeParams<QDateTime>::FGetter& getter) { return DateTimeParams<QDateTime>(getter); }

    TViewModelsColumnComponentsBuilder& AddTimeColumn(qint32 column, const FTranslationHandler& header, const std::function<QTime& (ValueType)>& getter){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TimeToString(getter(data));
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{ getter(data) = LanguageSettings::TimeFromVariant(value); };
        }, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return getter(data);
        });
    }

    TViewModelsColumnComponentsBuilder& AddTimePropertyColumn(qint32 column, const FTranslationHandler& header, const DateTimeParams<LocalPropertyDateTime>& params){
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
                    property = QDateTime(property.Native().date(), LanguageSettings::TimeFromVariant(value), Qt::OffsetFromUTC, *params.TimeShift);
                    return;
                }
                property = QDateTime(property.Native().date(), LanguageSettings::TimeFromVariant(value));
            };
        }, [params](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(params.TimeShift != nullptr) {
                return params.Getter(data).Native().toOffsetFromUtc(*params.TimeShift).time();
            }
            return params.Getter(data).Native().time();
        });
    }
    TViewModelsColumnComponentsBuilder& AddDateTimeColumn(qint32 column, const FTranslationHandler& header, const DateTimeParams<QDateTime>& params){
        return AddColumn(column, header, [params](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(params.TimeShift != nullptr) {
                auto dateTime = params.Getter(data);
                return DateTimeToString(dateTime.toOffsetFromUtc(*params.TimeShift));
            }
            return DateTimeToString(params.Getter(data));
        }, params.IsReadOnly ? FModelSetter() : [params](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                if(params.TimeShift != nullptr) {
                    auto& property = params.Getter(data);
                    auto inputDateTime = LanguageSettings::DateTimeFromVariant(value);
                    property = QDateTime(inputDateTime.date(), inputDateTime.time(), Qt::OffsetFromUTC, *params.TimeShift);
                    return;
                }
                params.Getter(data) = LanguageSettings::DateTimeFromVariant(value);
        };
        }, [params](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(params.TimeShift != nullptr) {
                auto dateTime = params.Getter(data);
                return dateTime.toOffsetFromUtc(*params.TimeShift);
            }
            return params.Getter(data);
        });
    }

    TViewModelsColumnComponentsBuilder& AddDateTimePropertyColumn(qint32 column, const FTranslationHandler& header, const DateTimeParams<LocalPropertyDateTime>& params){
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
                    auto inputDateTime = LanguageSettings::DateTimeFromVariant(value);
                    property = QDateTime(inputDateTime.date(), inputDateTime.time(), Qt::OffsetFromUTC, *params.TimeShift);
                    return;
                }
                params.Getter(data) = LanguageSettings::DateTimeFromVariant(value);
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

    using FDoubleOptGetterConst = std::function<std::optional<double>(ConstValueType)>;
    using FDoubleGetterConst = std::function<double(ConstValueType)>;
    using FDoubleSetter = std::function<void (ValueType, double)>;
    using FDoubleOptSetter = std::function<void (ValueType, const std::optional<double>&)>;

    TViewModelsColumnComponentsBuilder& AddMeasurementColumnLimits(const FDoubleGetterConst& min = [](ConstValueType){ return std::numeric_limits<double>().lowest(); }, const FDoubleGetterConst& max = [](ConstValueType){ return (std::numeric_limits<double>::max)(); })
    {
        return addMeasurementLimits([this](qint32 role, qint32 column, const ViewModelsTableColumnComponents::ColumnComponentData& data) {
            bool installedComponent = m_viewModel->ColumnComponents.SetComponent(role, column, 0, data);
            Q_ASSERT(installedComponent);
        }, min, max);
    }

    void AttachDependencies()
    {
        if(!m_currentMeasurementColumns.isEmpty()) {
            m_viewModel->AttachDependence(CONNECTION_DEBUG_LOCATION, &m_currentMeasurement->OnChanged, m_currentMeasurementColumns.First(), m_currentMeasurementColumns.Last());
            m_currentMeasurementColumns.clear();
        }
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumnCalculable(qint32 column, const FTranslationHandler& header, const std::function<double (ConstValueType)>& getter, const FDoubleSetter& setter = nullptr)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return setMeasurmentUnit(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto concreteValue = getter(const_cast<ValueType>(value));
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return DASH;
            }
            return pMeasurement->FromBaseToUnitUi(concreteValue);
        }, setter == nullptr ? FModelSetter() : [setter, pMeasurement, getter](const QVariant& data, ValueType value) -> FAction {
            auto toSet = pMeasurement->FromUnitToBase(data.toDouble());
            auto old = getter(value);
            if(!fuzzyCompare(old, toSet, pMeasurement->CurrentEpsilon)) {
                return [&value, toSet, setter]{
                    setter(value, toSet);
                };
            }
            return nullptr;
        }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(value)));
        }).addMeasurementLimits([this](qint32 role, qint32 column, const ViewModelsTableColumnComponents::ColumnComponentData& data){
            m_viewModel->ColumnComponents.AddComponent(role, column, data);
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumnCalculableOpt(qint32 column, const FTranslationHandler& header, const FDoubleOptGetterConst& getter, FDoubleOptSetter setter = nullptr)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return setMeasurmentUnit(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto dataValue = getter(const_cast<ValueType>(value));
            if(!dataValue.has_value()) {
                return DASH;
            }
            const auto& concreteValue = dataValue.value();
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return DASH;
            }

            return pMeasurement->FromBaseToUnitUi(concreteValue);
        }, setter == nullptr ? FModelSetter() : [setter, getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            bool isDouble; auto dval = data.toDouble(&isDouble);
            auto currentValue = getter(value);
            if(isDouble){
                auto toSet = pMeasurement->FromUnitToBase(dval);
                if(currentValue.has_value() && fuzzyCompare(currentValue.value(), toSet, pMeasurement->CurrentEpsilon)) {
                    return nullptr;
                }
                return [toSet, &value, setter]{ setter(value, toSet); };
            }
            if(!currentValue.has_value()) {
                return nullptr;
            }
            return [&]{ setter(value, std::nullopt); };
        }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto dataValue = getter(const_cast<ValueType>(value));
            if(!dataValue.has_value()) {
                return QVariant();
            }
            return pMeasurement->FromBaseToUnit(dataValue.value());
        }).addMeasurementLimits([this](qint32 role, qint32 column, const ViewModelsTableColumnComponents::ColumnComponentData& data){
              m_viewModel->ColumnComponents.AddComponent(role, column, data);
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<double& (ValueType)>& getter, bool readOnly = false)
    {
        return AddMeasurementColumnCalculable(column, header, [getter](ConstValueType value) {
            return getter(const_cast<ValueType>(value));
        }, readOnly ? FDoubleSetter() : [getter](ValueType data, double value) {
            getter(data) = value;
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column,
                                                             const FTranslationHandler& header,
                                                             const std::function<std::optional<double>& (ValueType)>& getter, bool readOnly = false)
    {
        return AddMeasurementColumnCalculableOpt(column, header, [getter](ConstValueType value) {
            return getter(const_cast<ValueType>(value));
        }, readOnly ? FDoubleOptSetter() : [getter](ValueType data, const std::optional<double>& value) {
            getter(data) = value;
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDouble& (ValueType)>& getter, bool readOnly = false)
    {
        return AddMeasurementColumnCalculable(column, header, [getter](ConstValueType value) {
            return getter(const_cast<ValueType>(value)).Native();
        }, readOnly ? FDoubleSetter() : [getter](ValueType data, double value) {
            getter(data) = value;
        }).AddMeasurementColumnLimits([getter](ConstValueType v) { return getter(const_cast<ValueType>(v)).GetMin(); }, [getter](ConstValueType v) { return getter(const_cast<ValueType>(v)).GetMax(); });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDoubleOptional& (ValueType)>& getter, bool readOnly = false)
    {
        return AddMeasurementColumnCalculableOpt(column, header, [getter](ConstValueType value) {
            return getter(const_cast<ValueType>(value)).Native();
        }, readOnly ? FDoubleOptSetter() : [getter](ValueType data, const std::optional<double>& value) {
            getter(data) = value;
        }).AddMeasurementColumnLimits([getter](ConstValueType v) { return getter(const_cast<ValueType>(v)).Value.GetMin(); }, [getter](ConstValueType v) { return getter(const_cast<ValueType>(v)).Value.GetMax(); });
    }

private:
    static QString setMeasurmentUnit(const QString& string, const Measurement* measurment);
    TViewModelsColumnComponentsBuilder& addMeasurementLimits(const std::function<void (qint32, qint32, const ViewModelsTableColumnComponents::ColumnComponentData&)>& addDelegate,
                                                             const FDoubleGetterConst& min = [](ConstValueType){ return std::numeric_limits<double>().lowest(); },
                                                             const FDoubleGetterConst& max = [](ConstValueType){ return (std::numeric_limits<double>::max)(); });

#endif
private:
    FModelModelGetter m_modelGetter;
    qint32 m_currentColumn;
#ifdef UNITS_MODULE_LIB
    const class Measurement* m_currentMeasurement;
    Array<qint32> m_currentMeasurementColumns;
#endif
};

template<class T>
class TViewModelsColumnComponentsStandardBuilder : public TViewModelsColumnComponentsBuilder<T>
{
    using Super = TViewModelsColumnComponentsBuilder<T>;
public:
    using Super::Super;
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
