#ifndef VIEWMODELSDEFAULTCOMPONENTS_H
#define VIEWMODELSDEFAULTCOMPONENTS_H

#include "modelstablebase.h"

template<class T> class ModelsErrorComponent;

class ViewModelsDefaultComponents : public QObject
{
    using Super = QObject;
public:
    ViewModelsDefaultComponents(ViewModelsTableBase* model)
        : Super(model)
        , m_model(model)
    {}

    void ApplyDefaultColors(LocalPropertyColor* enabledCellColor, LocalPropertyColor* disabledCellColor,
                                   LocalPropertyColor* enabledTextColor, LocalPropertyColor* disabledTextColor);

    template<class Wrapper>
    void ApplyErrorComponent(ModelsErrorComponent<Wrapper>* component, const std::map<qint32, QVector<qint64>>& columns)
    {
        for(const auto& [column, errorsStack] : columns) {
            auto errorsStackCopy = errorsStack;
            m_model->ColumnComponents.AddComponent(Qt::DecorationRole, column, ViewModelsTableColumnComponents::ColumnComponentData()
                                                   .SetGetter([this, component, errorsStackCopy](const QModelIndex& index, bool&) {
                                                        const auto& data = m_model->GetData().Cast<Wrapper>()->At(index.row());
                                                        return component->ErrorIcon(data, errorsStackCopy, m_model->GetIconsContext());
                                                    }));
            m_model->ColumnComponents.AddComponent(Qt::ToolTipRole, column, ViewModelsTableColumnComponents::ColumnComponentData()
                                                   .SetGetter([this, component, errorsStackCopy](const QModelIndex& index, bool&) {
                                                        const auto& data = m_model->GetData().Cast<Wrapper>()->At(index.row());
                                                        return component->ErrorString(data, errorsStackCopy);
                                                    }));
        }
    }

#ifdef UNITS_MODULE_LIB
    struct ApplyUnitsComponentHandlers
    {
        std::function<double (qint32 row, bool& hasValue)> ValueGetter;
        std::function<bool (qint32 row, double value)> ValueSetter;
        FTranslationHandler Header;

        ApplyUnitsComponentHandlers(){}
        ApplyUnitsComponentHandlers(const FTranslationHandler& header, const std::function<double (qint32 row, bool&)>& getter,
                           const std::function<bool (qint32 row, double value)>& setter)
            : ValueGetter(getter)
            , ValueSetter(setter)
            , Header(header)
        {}
    };

    class ApplyUnitsComponentColumnHandlers : public QMap<qint32, ApplyUnitsComponentHandlers>
    {
    public:
        ApplyUnitsComponentColumnHandlers& AddColumn(qint32 column, const ApplyUnitsComponentHandlers& handlers)
        {
            insert(column, handlers);
            return *this;
        }
    };

    class ApplyUnitsComponentParams : public std::map<Name, ApplyUnitsComponentColumnHandlers>
    {
    public:
        ApplyUnitsComponentColumnHandlers& AddMeasurement(const Name& name)
        {
            Q_ASSERT(find(name) == end());
            return operator[](name);
        }
    };


    void ApplyUnitsComponent(const ApplyUnitsComponentParams& map);

    template<class Wrapper>
    static ApplyUnitsComponentHandlers UnitHandlersFromModelTable(const FTranslationHandler& header, Wrapper* model, const std::function<double& (typename Wrapper::value_type&)>& targetField)
    {
        using value_type = typename Wrapper::value_type;
        ApplyUnitsComponentHandlers result;
        result.ValueGetter = [model, targetField](qint32 row, bool& hasValue) {
            if(row >= model->GetSize()) {
                hasValue = false;
                return 0.0;
            }
            return targetField(model->EditSilent()[row]);
        };

        result.ValueSetter = [model, targetField](qint32 row, double value) {
            if(row >= model->GetSize()) {
                return false;
            }
            return model->EditWithCheck(row, [value, targetField](value_type& data){
                return [&]{ targetField(data) = value; };
            });
        };

        result.Header = header;
        return result;
    }

#endif

private:
    ViewModelsTableBase* m_model;
    DispatcherConnectionsSafe m_connections;
};
#endif // VIEWMODELSDEFAULTCOMPONENTS_H
