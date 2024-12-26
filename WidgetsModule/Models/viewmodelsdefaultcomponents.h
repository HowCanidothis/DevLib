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

enum ViewModelsColorFormatingRuleInitialization
{
    MinOnly,
    MaxOnly
};

template<class Property>
struct TViewModelsColorFormatingRule
{
    using value_type = typename Property::value_type;
    using Initializer = LocalPropertyDescInitializationParams<value_type>;
    using FCompare = std::function<bool (value_type, value_type)>;

    LocalPropertyBool Enabled;
    LocalPropertyBool EnableMin;
    LocalPropertyBool EnableMax;
    LocalPropertyColor LessColor;
    LocalPropertyColor GreaterColor;
    LocalPropertyColor NormalColor;
    Property Min;
    Property Max;

    DispatchersCommutator OnChanged;

    TViewModelsColorFormatingRule(const Initializer& initializer, ViewModelsColorFormatingRuleInitialization initialization)
        : TViewModelsColorFormatingRule(initialization == MinOnly ? initializer : Initializer(), initialization == MinOnly ? Initializer() : initializer)
    {
        if(initialization == MinOnly) {
            EnableMax.EditSilent() = false;
        } else {
            EnableMin.EditSilent() = false;
        }
    }

    TViewModelsColorFormatingRule(const Initializer& minInitializer, const Initializer& maxInitializer = Initializer())
        : EnableMin(true)
        , EnableMax(true)
        , LessColor(SharedSettings::GetInstance().StyleSettings.DefaultRedColor.Native())
        , GreaterColor(SharedSettings::GetInstance().StyleSettings.DefaultRedColor.Native())
        , NormalColor(SharedSettings::GetInstance().StyleSettings.DefaultGreenColor.Native())
        , Min(minInitializer)
        , Max(maxInitializer)
    {
        OnChanged.ConnectFrom(CDL, Enabled, EnableMin, EnableMax, LessColor, GreaterColor, NormalColor, Min, Max);
    }
};

using ViewModelsColorFormatingRuleDouble = TViewModelsColorFormatingRule<LocalPropertyDouble>;
using ViewModelsColorFormatingRuleInt = TViewModelsColorFormatingRule<LocalPropertyInt>;

template<class Wrapper, typename ValueType = typename ReferenceHelper<typename Wrapper::value_type>::reference, typename ConstValueType = typename ReferenceHelper<typename Wrapper::value_type>::const_reference>
class TViewModelsColumnComponentsBuilder : public TViewModelsColumnComponentsBuilderBase
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

    template<class T, typename FCompare = typename TViewModelsColorFormatingRule<T>::FCompare>
    TViewModelsColumnComponentsBuilder& SetTextColorFormatingRule(qint32 column, const TViewModelsColorFormatingRule<T>* rule, const FCompare& compare)
    {
        auto* viewModel = m_viewModel;
        m_viewModel->ColumnComponents.AddComponent(Qt::ForegroundRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetGetter([compare, viewModel, rule](const QModelIndex& index) -> std::optional<QVariant> {
            if(!rule->Enabled) {
                return std::nullopt;
            }
            auto v = index.data(Qt::EditRole).toDouble();
#ifdef UNITS_MODULE_LIB
            auto* measurement = index.model()->headerData(index.column(), Qt::Horizontal, UnitRole).value<const Measurement*>();
            if(measurement != nullptr) {
                v = measurement->FromUnitToBase(v);
            }
#endif
            if(rule->EnableMin) {
                if(compare(v, rule->Min)) {
                   return rule->NormalColor.Native();
                }
                if(v < rule->Min) {
                    return rule->LessColor.Native();
                }
            }
            if(rule->EnableMax) {
                if(compare(rule->Max, v)) {
                    return rule->NormalColor.Native();
                }
                if(v > rule->Max) {
                    return rule->GreaterColor.Native();
                }
            }
            return rule->NormalColor.Native();
        }));
        m_viewModel->AttachDependence(CDL, &rule->OnChanged, column, column);
        return *this;
    }

    TViewModelsColumnComponentsBuilder& SetTextColorFormatingRule(qint32 column, const std::function<std::optional<QVariant>(ConstValueType)>& handler)
    {
        auto modelGetter = m_modelGetter;
        m_viewModel->ColumnComponents.AddComponent(Qt::ForegroundRole, column, ViewModelsTableColumnComponents::ColumnComponentData().SetGetter([handler, modelGetter](const QModelIndex& index){
            ConstValueType data = modelGetter()->At(index.row());
            return handler(data);
        }));
        return *this;
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


    template<typename T>
    TViewModelsColumnComponentsBuilder& AddColumn(qint32 column, const FTranslationHandler& header, const std::function<T (ConstValueType)>& getter) {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            return getter(constData);
        });
    }

    template<typename T>
    TViewModelsColumnComponentsBuilder& AddColumn(qint32 column, const FTranslationHandler& header, const std::function<T& (ValueType)>& getter) {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType& data = const_cast<ValueType>(constData);
            return getter(data);
        }, [getter](const QVariant& v, ValueType data) {
            return [&]{ getter(data) = v.value<T>(); };
        });
    }

    template<typename T>
    TViewModelsColumnComponentsBuilder& AddColumn(qint32 column, const FTranslationHandler& header, const std::function<std::optional<T> (ConstValueType)>& getter) {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            auto v = getter(constData);
            if(!v.has_value()) {
                return DASH;
            }
            return v.value();
        });
    }

    template<typename Optional, typename T = typename Optional::value_type>
    typename std::enable_if<std::is_same<std::optional<T>, Optional>::value, TViewModelsColumnComponentsBuilder&>::type AddColumn(qint32 column, const FTranslationHandler& header, const std::function<Optional& (ValueType)>& getter) {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType& data = const_cast<ValueType>(constData);
            auto& v = getter(data);
            if(!v.has_value()) {
                return DASH;
            }
            return v.value();
        }, [getter](const QVariant& v, ValueType data) {
            return [&]{
                auto& ref = getter(data);
                if(v.isValid()) {
                    ref = v.value<T>();
                } else {
                    ref = std::nullopt;
                }
            };
        });
    }

    template<typename Property, typename T = typename Property::value_type>
    typename std::enable_if<std::is_base_of<LocalProperty<T>, Property>::value, TViewModelsColumnComponentsBuilder&>::type AddColumn(qint32 column, const FTranslationHandler& header, const std::function<Property& (ValueType)>& getter) {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType& data = const_cast<ValueType>(constData);
            const auto& v = getter(data).Native();
            return v;
        }, [getter](const QVariant& v, ValueType data) {
            return [&]{
                auto& ref = getter(data);
                ref = v.value<T>();
            };
        });
    }

    template<typename Property, typename T = typename Property::value_type>
    typename std::enable_if<std::is_base_of<LocalProperty<T>, Property>::value, TViewModelsColumnComponentsBuilder&>::type AddColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyOptional<Property>& (ValueType)>& getter) {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType& data = const_cast<ValueType>(constData);
            const auto& v = getter(data).Native();
            if(!v.has_value()) {
                return DASH;
            }
            return v.value();
        }, [getter](const QVariant& v, ValueType data) {
            return [&]{
                auto& ref = getter(data);
                if(v.isValid()) {
                    ref = v.value<T>();
                } else {
                    ref = std::nullopt;
                }
            };
        });
    }

#define ViewModelsColumnComponentsBuilder_DECLARE_TYPE(name, type, property) \
    TViewModelsColumnComponentsBuilder& Add##name##ByRef(qint32 column, const FTranslationHandler& header, const std::function<property& (ValueType)>& getter) \
    { \
        return AddColumn<property, property::value_type>(column, header, getter); \
    } \
    TViewModelsColumnComponentsBuilder& Add##name##ByRefOpt(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyOptional<property>& (ValueType)>& getter) \
    { \
        return AddColumn<property, property::value_type>(column, header, getter); \
    } \
    TViewModelsColumnComponentsBuilder& Add##name##ByRef(qint32 column, const FTranslationHandler& header, const std::function<type& (ValueType)>& getter) \
    { \
        return AddColumn<type>(column, header, getter); \
    } \
    TViewModelsColumnComponentsBuilder& Add##name(qint32 column, const FTranslationHandler& header, const std::function<type (ConstValueType)>& getter) \
    { \
        return AddColumn<type>(column, header, getter); \
    } \
    TViewModelsColumnComponentsBuilder& Add##name##ByRefOpt(qint32 column, const FTranslationHandler& header, const std::function<std::optional<type>& (ValueType)>& getter) \
    { \
        return AddColumn<std::optional<type>, type>(column, header, getter); \
    } \
    TViewModelsColumnComponentsBuilder& Add##name##Opt(qint32 column, const FTranslationHandler& header, const std::function<std::optional<type> (ConstValueType)>& getter) \
    { \
        return AddColumn<type>(column, header, getter); \
    }

    ViewModelsColumnComponentsBuilder_DECLARE_TYPE(String, QString, LocalPropertyString)
    ViewModelsColumnComponentsBuilder_DECLARE_TYPE(Name, Name, LocalPropertyName)
    ViewModelsColumnComponentsBuilder_DECLARE_TYPE(Color, QColor, LocalPropertyColor)
    ViewModelsColumnComponentsBuilder_DECLARE_TYPE(Double, double, LocalPropertyDouble)
    ViewModelsColumnComponentsBuilder_DECLARE_TYPE(Int, qint32, LocalPropertyInt)
    ViewModelsColumnComponentsBuilder_DECLARE_TYPE(Bool, bool, LocalPropertyBool)


    template<class Enum>
    TViewModelsColumnComponentsBuilder& AddEnum(qint32 column, const FTranslationHandler& header, const std::function<Enum (ConstValueType)>& getter)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TranslatorManager::ToVariant<Enum>(getter(data));
        });
    }

    template<class Enum>
    TViewModelsColumnComponentsBuilder& AddEnum(qint32 column, const FTranslationHandler& header, const std::function<qint32 (ConstValueType)>& getter)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TranslatorManager::ToVariant<Enum>(getter(data));
        });
    }

    template<class Enum>
    TViewModelsColumnComponentsBuilder& AddEnumByRef(qint32 column, const FTranslationHandler& header, const std::function<Enum& (ValueType)>& getter)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TranslatorManager::ToVariant<Enum>(getter(data));
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            return TranslatorManager::SetterFromString(getter(data), value.toString());
        });
    }

    template<class Enum>
    TViewModelsColumnComponentsBuilder& AddEnumByRef(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertySequentialEnum<Enum>& (ValueType)>& getter)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TranslatorManager::ToVariant<Enum>(getter(data));
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            return getter(data).SetterFromString(value.toString());
        });
    }

    TViewModelsColumnComponentsBuilder& AddDateByRef(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDate& (ValueType)>& getter){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            LocalPropertyDate& property = getter(data);
            return property.Native().isValid() ? DateToString(property.Native()) : QVariant("-");
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                LocalPropertyDate& property = getter(data);
                property = DateFromVariant(value);
            };
        }/*, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            LocalPropertyDate& property = getter(data);
            return property.Native();
        }*/);
    }

    TViewModelsColumnComponentsBuilder& AddDateByRef(qint32 column, const FTranslationHandler& header, const std::function<QDate& (ValueType)>& getter){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            QDate& property = getter(data);
            return property.isValid() ? DateToString(property) : QVariant("-");
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                QDate& property = getter(data);
                property = DateFromVariant(value);
            };
        }/*, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            QDate& property = getter(data);
            return property;
        }*/);
    }

    TViewModelsColumnComponentsBuilder& AddDate(qint32 column, const FTranslationHandler& header, const std::function<QDate (ConstValueType)>& getter){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            QDate property = getter(data);
            return property.isValid() ? DateToString(property) : QVariant("-");
        }, FModelSetter()/*, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            QDate& property = getter(data);
            return property;
        }*/);
    }

    TViewModelsColumnComponentsBuilder& AddTimeByRef(qint32 column, const FTranslationHandler& header, const std::function<QTime& (ValueType)>& getter){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TimeToString(getter(data));
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{ getter(data) = TimeFromVariant(value); };
        }, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return getter(data);
        });
    }

    TViewModelsColumnComponentsBuilder& AddTimeByRef(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyTime& (ValueType)>& getter){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TimeToString(getter(data));
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            return [&]{ getter(data) = TimeFromVariant(value); };
        }, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return getter(data);
        });
    }

    TViewModelsColumnComponentsBuilder& AddTime(qint32 column, const FTranslationHandler& header, const std::function<QTime (ConstValueType)>& getter){
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            return TimeToString(getter(constData));
        }, FModelSetter(), [getter](ConstValueType constData)-> QVariant {
            return getter(constData);
        });
    }

    TViewModelsColumnComponentsBuilder& AddDateTime(qint32 column, const FTranslationHandler& header, const std::function<QDateTime (ConstValueType)>& getter, const double* timeShift = nullptr){
        return AddColumn(column, header, [getter, timeShift](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            QDateTime dateTime = getter(data);
            if(timeShift != nullptr) {
                dateTime = dateTime.toOffsetFromUtc(*timeShift);
            }
            return LanguageSettings::DateTimeToString(dateTime);
        }, FModelSetter(), [getter, timeShift](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(timeShift != nullptr) {
                QDateTime dateTime = getter(data);
                return dateTime.toOffsetFromUtc(*timeShift);
            }
            return getter(data);
        });
    }

    TViewModelsColumnComponentsBuilder& AddDateTimeByRef(qint32 column, const FTranslationHandler& header, const std::function<QDateTime& (ValueType)>& getter, const double* timeShift = nullptr){
        return AddColumn(column, header, [getter, timeShift](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(timeShift != nullptr) {
                QDateTime& dateTime = getter(data);
                return DateTimeToString(dateTime.toOffsetFromUtc(*timeShift));
            }
            return DateTimeToString(getter(data));
        }, [getter, timeShift](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                if(timeShift != nullptr) {
                    QDateTime& property = getter(data);
                    auto inputDateTime = DateTimeFromVariant(value);
                    property = QDateTime(inputDateTime.date(), inputDateTime.time(), Qt::OffsetFromUTC, *timeShift);
                    return;
                }
                getter(data) = DateTimeFromVariant(value);
        };
        }, [getter, timeShift](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(timeShift != nullptr) {
                QDateTime& dateTime = getter(data);
                return dateTime.toOffsetFromUtc(*timeShift);
            }
            return getter(data);
        });
    }

    TViewModelsColumnComponentsBuilder& AddDateTimeByRef(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDateTime& (ValueType)>& getter, const double* timeShift = nullptr){
        return AddColumn(column, header, [getter, timeShift](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(timeShift != nullptr) {
                LocalPropertyDateTime& dateTime = getter(data);
                return DateTimeToString(dateTime.Native().toOffsetFromUtc(*timeShift));
            }
            return DateTimeToString(getter(data));
        }, [getter, timeShift](const QVariant& value, ValueType data) -> FAction {
            return [&]{
                if(timeShift != nullptr) {
                    LocalPropertyDateTime& property = getter(data);
                    auto inputDateTime = DateTimeFromVariant(value);
                    property = QDateTime(inputDateTime.date(), inputDateTime.time(), Qt::OffsetFromUTC, *timeShift);
                    return;
                }
                getter(data) = DateTimeFromVariant(value);
        };
        }, [getter, timeShift](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            if(timeShift != nullptr) {
                LocalPropertyDateTime& dateTime = getter(data);
                return dateTime.Native().toOffsetFromUtc(*timeShift);
            }
            return getter(data);
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
        return AddColumn(column, [header, pMeasurement]{ return MeasurementManager::MakeMeasurementString(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
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
        return AddColumn(column, [header, pMeasurement]{ return MeasurementManager::MakeMeasurementString(header(), pMeasurement); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
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
    TViewModelsColumnComponentsBuilder& addMeasurementLimits(const std::function<void (qint32, qint32, const ViewModelsTableColumnComponents::ColumnComponentData&)>& addDelegate,
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
