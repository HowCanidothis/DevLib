#ifndef VIEWMODELSDEFAULTCOMPONENTS_H
#define VIEWMODELSDEFAULTCOMPONENTS_H

#include "modelstablebase.h"

template<class T> class ModelsErrorComponent;

class ViewModelsColumnComponentsBuilderBase
{
    using Super = QObject;
public:
    ViewModelsColumnComponentsBuilderBase(ViewModelsTableBase* model)
        : m_viewModel(model)
    {}

    ViewModelsColumnComponentsBuilderBase& AddDefaultColors(LocalPropertyColor* enabledCellColor, LocalPropertyColor* disabledCellColor,
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

    operator T*() const { return m_value; }

private:
    T* m_value;
};

template<class Wrapper, typename ValueType = typename Wrapper::value_type&, typename ConstValueType = const typename Wrapper::value_type&>
class ViewModelsColumnComponentsBuilder : ViewModelsColumnComponentsBuilderBase
{
    using Super = ViewModelsColumnComponentsBuilderBase;

public:
    using FModelGetter = std::function<QVariant (ConstValueType)>;
    using FModelSetter = std::function<FAction (const QVariant& data, ValueType)>;

    ViewModelsColumnComponentsBuilder(ViewModelsTableBase* viewModel, const std::function<Wrapper* ()>& modelGetter)
        : Super(viewModel)
        , m_modelGetter(modelGetter)
    {}

    ~ViewModelsColumnComponentsBuilder()
    {
#ifdef UNITS_MODULE_LIB
        AttachDependencies();
#endif
    }

    ViewModelsColumnComponentsBuilder& AddDefaultColors()
    {
        auto& settings = SharedSettings::GetInstance();
        Super::AddDefaultColors(&settings.StyleSettings.EnabledTableCellColor, &settings.StyleSettings.DisabledTableCellColor,
                         &settings.StyleSettings.EnabledTableCellTextColor, &settings.StyleSettings.DisabledTableCellTextColor);
        return *this;
    }

    ViewModelsColumnComponentsBuilder& AddErrorComponent(ModelsErrorComponent<Wrapper>* component, const std::map<qint32, QVector<qint64>>& columns)
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

    ViewModelsColumnComponentsBuilder& AddColumn(qint32 column, const FTranslationHandler& header, const FModelGetter& getterUi, const FModelSetter& setter = nullptr, const FModelGetter& inGetter = nullptr)
    {
        auto modelGetter = m_modelGetter;
        ViewModelsTableColumnComponents::ColumnComponentData displayRoleComponent;
        auto getter = inGetter;
        if(getter == nullptr) {
            getter = getterUi;
        }

        displayRoleComponent.GetterHandler = [modelGetter, getterUi](const QModelIndex& index) -> std::optional<QVariant> {
            if(index.row() >= modelGetter()->GetSize()) {
                return "-";
            }
            return getterUi(modelGetter()->At(index.row()));
        };
        displayRoleComponent.GetHeaderHandler = [header]{ return header(); };

        m_viewModel->ColumnComponents.AddComponent(Qt::DisplayRole, column, displayRoleComponent);

        auto editRoleGetter = [modelGetter, getter](const QModelIndex& index) -> std::optional<QVariant> {
            if(index.row() >= modelGetter()->GetSize()) {
                return QVariant();
            }
            return getter(modelGetter()->At(index.row()));
        };

        ViewModelsTableColumnComponents::ColumnComponentData editRoleComponent;
        editRoleComponent.GetterHandler = editRoleGetter;

        if(setter != nullptr) {
            editRoleComponent.SetterHandler = [modelGetter, setter](const QModelIndex& index, const QVariant& data) -> std::optional<bool> {
                if(index.row() >= modelGetter()->GetSize()) {
                    return false;
                }
                return modelGetter()->EditWithCheck(index.row(), [&](ValueType value){ return setter(data, value); });
            };

            m_viewModel->ColumnComponents.AddFlagsComponent(column, { [](qint32) { return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable; } });
        } else {
            m_viewModel->ColumnComponents.AddFlagsComponent(column, { [](qint32) { return Qt::ItemIsEnabled | Qt::ItemIsSelectable; } });
        }

        m_viewModel->ColumnComponents.AddComponent(Qt::EditRole, column, editRoleComponent);

        return *this;
    }

    template<class Enum>
    ViewModelsColumnComponentsBuilder& AddEnumColumn(qint32 column, const FTranslationHandler& header, const std::function<Enum& (ValueType)>& getter)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TranslatorManager::IsValid<Enum>(getter(data)) ? TranslatorManager::GetNames<Enum>()[static_cast<int>(getter(data))] : QVariant();
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            auto enumIndex = TranslatorManager::GetNames<Enum>().indexOf(value.toString());
            if(TranslatorManager::IsValid<Enum>(enumIndex)){
                LambdaValueWrapper<ValueType> wrapper(data);
                return [enumIndex, wrapper, getter]{ getter(wrapper) = static_cast<Enum>(enumIndex); };
            }
            return nullptr;
        });
    }

    template<class Enum>
    ViewModelsColumnComponentsBuilder& AddEnumPropertyColumn(qint32 column, const FTranslationHandler& header, const std::function<LocalPropertySequentialEnum<Enum>& (ValueType)>& getter)
    {
        return AddColumn(column, header, [getter](ConstValueType constData)-> QVariant {
            ValueType data = const_cast<ValueType>(constData);
            return TranslatorManager::GetNames<Enum>()[static_cast<int>(getter(data).Native())];
        }, [getter](const QVariant& value, ValueType data) -> FAction {
            auto enumIndex = TranslatorManager::GetNames<Enum>().indexOf(value.toString());
            if(TranslatorManager::IsValid<Enum>(enumIndex)){
                LambdaValueWrapper<ValueType> wrapper(data);
                return [enumIndex, wrapper, getter]{ getter(wrapper) = static_cast<Enum>(enumIndex); };
            }
            return nullptr;
        });
    }

#ifdef UNITS_MODULE_LIB
    ViewModelsColumnComponentsBuilder& SetCurrentMeasurement(const Name& measurementName)
    {
        AttachDependencies();
        m_currentMeasurement = MeasurementManager::GetInstance().GetMeasurement(measurementName);
        return *this;
    }

    void AttachDependencies()
    {
        if(!m_currentMeasurementColumns.isEmpty()) {
            m_viewModel->AttachDependence(&m_currentMeasurement->OnChanged, m_currentMeasurementColumns.First(), m_currentMeasurementColumns.Last());
            m_currentMeasurementColumns.clear();
        }
    }

    ViewModelsColumnComponentsBuilder& AddMeasurementColumnCalculable(qint32 column, const FTranslationHandler& header, const std::function<double (ValueType)>& getter)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement.get();
        return AddColumn(column, [header, pMeasurement]{ return header().arg(pMeasurement->CurrentUnitLabel); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            return QString::number(pMeasurement->BaseValueToCurrentUnit(getter(const_cast<ValueType>(value))), 'f', pMeasurement->CurrentPrecision);
        }, FModelSetter(), [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->BaseValueToCurrentUnit(getter(const_cast<ValueType>(value)));
        });
    }

    ViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<double& (ValueType)>& getter, bool readOnly = false)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement.get();
        return AddColumn(column, [header, pMeasurement]{ return header().arg(pMeasurement->CurrentUnitLabel); }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            return QString::number(pMeasurement->BaseValueToCurrentUnit(getter(const_cast<ValueType>(value))), 'f', pMeasurement->CurrentPrecision);
        }, readOnly ? FModelSetter() : [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{ getter(value) = pMeasurement->CurrentUnitToBaseValue(data.toDouble()); };
        }, [getter, pMeasurement](ConstValueType value) -> QVariant {
            return pMeasurement->BaseValueToCurrentUnit(getter(const_cast<ValueType>(value)));
        });
    }

    ViewModelsColumnComponentsBuilder& AddMeasurementColumn(qint32 column, const FTranslationHandler& header, const std::function<std::optional<double>& (ValueType)>& getter, bool readOnly = false)
    {
        Q_ASSERT(m_currentMeasurement != nullptr);
        Q_ASSERT(m_currentMeasurementColumns.FindSorted(column) == m_currentMeasurementColumns.end());
        m_currentMeasurementColumns.InsertSortedUnique(column);

        auto pMeasurement = m_currentMeasurement.get();
        return AddColumn(column, [header, pMeasurement]{ return header().arg(pMeasurement->CurrentUnitLabel); }, [getter, pMeasurement](ConstValueType constValue) -> QVariant {
            auto& value = const_cast<ValueType>(constValue);
            auto& dataValue = getter(value);
            if(!dataValue.has_value()) {
                return "-";
            }
            return QString::number(pMeasurement->BaseValueToCurrentUnit(dataValue.value()), 'f', pMeasurement->CurrentPrecision);
        }, readOnly ? FModelSetter() : [getter, pMeasurement](const QVariant& data, ValueType value) -> FAction {
            return [&]{
                if(data.isValid()) {
                    getter(value) = pMeasurement->CurrentUnitToBaseValue(data.toDouble());
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
            return pMeasurement->BaseValueToCurrentUnit(dataValue.value());
        });
    }
#endif
private:
    std::function<Wrapper* ()> m_modelGetter;
#ifdef UNITS_MODULE_LIB
    SharedPointer<class Measurement> m_currentMeasurement;
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
