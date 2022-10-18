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
struct LambdaValueWrapper
{

};

template<class T>
struct LambdaValueWrapper<T&>
{
    LambdaValueWrapper(T& value)
        : m_value(value)
    {}

    T& GetValue() const { return m_value; }
    operator T&() const { return m_value; }

private:
    T& m_value;
};

template<class T>
struct LambdaValueWrapper<T*>
{
    LambdaValueWrapper(T* value)
        : m_value(value)
    {}

    T* GetValue() const { return m_value; }
    operator T*() const { return m_value; }

private:
    T* m_value;
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

    TViewModelsColumnComponentsBuilder& AddErrorComponent(ModelsErrorComponent<Wrapper>* component, const std::map<qint32, QVector<qint64>>& columns)
    {
        auto* viewModel = m_viewModel;
        auto modelGetter = m_modelGetter;
        for(const auto& [column, errorsStack] : columns) {
            auto errorsStackCopy = errorsStack;
            viewModel->ColumnComponents.AddComponent(Qt::DecorationRole, column, ViewModelsTableColumnComponents::ColumnComponentData()
                                                   .SetGetter([viewModel, component, errorsStackCopy, modelGetter](const QModelIndex& index) {
                                                        ConstValueType data = modelGetter()->At(index.row());
                                                        return component->ErrorIcon(data, errorsStackCopy, viewModel->GetIconsContext());
                                                    }));
            viewModel->ColumnComponents.AddComponent(Qt::ToolTipRole, column, ViewModelsTableColumnComponents::ColumnComponentData()
                                                   .SetGetter([component, errorsStackCopy, modelGetter](const QModelIndex& index) {
                                                        ConstValueType data = modelGetter()->At(index.row());
                                                        return component->ErrorString(data, errorsStackCopy);
                                                    }));
        }
        return *this;
    }

    TViewModelsColumnComponentsBuilder& AddColumn(qint32 column, const FTranslationHandler& header, const FModelGetter& getterUi, const FModelSetter& setter = nullptr, const FModelGetter& inGetter = nullptr)
    {
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
            editRoleComponent.SetterHandler = [modelGetter, setter](const QModelIndex& index, const QVariant& data) -> std::optional<bool> {
                const auto& viewModel = modelGetter();
                if(viewModel == nullptr) {
                    return false;
                }
                if(index.row() >= viewModel->GetSize()) {
                    return false;
                }
                return viewModel->EditWithCheck(index.row(), [&](ValueType value){ return setter(data, value); });
            };

            m_viewModel->ColumnComponents.AddFlagsComponent(column, { [](qint32) { return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable; } });
        } else {
            m_viewModel->ColumnComponents.AddFlagsComponent(column, { [](qint32) { return Qt::ItemIsEnabled | Qt::ItemIsSelectable; } });
        }

        m_viewModel->ColumnComponents.AddComponent(Qt::EditRole, column, editRoleComponent);

        return *this;
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

#ifdef UNITS_MODULE_LIB
    TViewModelsColumnComponentsBuilder& SetCurrentMeasurement(const Measurement* measurement)
    {
        AttachDependencies();
        m_currentMeasurement = measurement;
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
        return AddColumn(column, [header, pMeasurement]{ return header().arg(pMeasurement->CurrentUnitLabel); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto concreteValue = getter(const_cast<ValueType>(value));
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }
            return QString::number(pMeasurement->FromBaseToUnit(concreteValue), 'f', pMeasurement->CurrentPrecision);
        }, FModelSetter(), [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(value)));
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumnCalculable(qint32 column, const FTranslationHandler& header, const std::function<std::optional<double>(ConstValueType)>& getter)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return header().arg(pMeasurement->CurrentUnitLabel); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto& dataValue = getter(const_cast<ValueType>(value));
            if(!dataValue.has_value()) {
                return "-";
            }
            const auto& concreteValue = dataValue.value();
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }

            return QString::number(pMeasurement->FromBaseToUnit(concreteValue), 'f', pMeasurement->CurrentPrecision);
        }, FModelSetter(), [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(value)));
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<double& (ValueType)>& getter, bool readOnly = false)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return header().arg(pMeasurement->CurrentUnitLabel); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto concreteValue = getter(const_cast<ValueType>(value));
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }
            return QString::number(pMeasurement->FromBaseToUnit(concreteValue), 'f', pMeasurement->CurrentPrecision);
        }, readOnly ? FModelSetter() : [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{ getter(value) = pMeasurement->FromUnitToBase(data.toDouble()); };
        }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(value)));
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertyDouble& (ValueType)>& getter, bool readOnly = false)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return header().arg(pMeasurement->CurrentUnitLabel); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            auto concreteValue = getter(const_cast<ValueType>(value)).Native();
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }
            return QString::number(pMeasurement->FromBaseToUnit(concreteValue), 'f', pMeasurement->CurrentPrecision);
        }, readOnly ? FModelSetter() : [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{ getter(value) = pMeasurement->FromUnitToBase(data.toDouble()); };
        }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->FromBaseToUnit(getter(const_cast<ValueType>(value)));
        });
    }

    TViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<std::optional<double>& (ValueType)>& getter, bool readOnly = false)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement;
        return AddColumn(column, [header, pMeasurement]{ return header().arg(pMeasurement->CurrentUnitLabel); }, [getter, pMeasurement](ConstValueType constValue) -> QVariant {
            auto& value = const_cast<ValueType>(constValue);
            auto& dataValue = getter(value);
            if(!dataValue.has_value()) {
                return "-";
            }
            const auto& concreteValue = dataValue.value();
            if(qIsNaN(concreteValue) || qIsInf(concreteValue)) {
                return "-";
            }
            return QString::number(pMeasurement->FromBaseToUnit(concreteValue), 'f', pMeasurement->CurrentPrecision);
        }, readOnly ? FModelSetter() : [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{
                if(data.isValid()) {
                    getter(value) = pMeasurement->FromUnitToBase(data.toDouble());
                } else {
                    getter(value) = std::nullopt;
                }
            };
        }, [getter, pMeasurement](ConstValueType constValue) -> QVariant {
            auto& value = const_cast<ValueType>(constValue);
            auto& dataValue = getter(value);
            if(!dataValue.has_value()) {
                return 0.0;
            }
            return pMeasurement->FromBaseToUnit(dataValue.value());
        });
    }
#endif
private:
    std::function<Wrapper* ()> m_modelGetter;
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
