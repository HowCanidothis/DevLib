#ifndef WIDGETSDECLARATIONS_H
#define WIDGETSDECLARATIONS_H

#include <QHash>
#include <QDialogButtonBox>

template<class T> class TViewModelsTableBase;
template<class T> class TViewModelsEditTable;

struct DescCustomDialogParams
{
    QVector<std::tuple<QDialogButtonBox::ButtonRole, QString, FAction>> Buttons;
    QWidget* View = nullptr;
    qint32 DefaultButtonIndex = 0;
    bool DefaultSpacing = true;
    FAction OnRejected = []{};
    FAction OnAccepted = []{};

    DescCustomDialogParams& SetOnRejected(const FAction& handler) { OnRejected = handler; return *this; }
    DescCustomDialogParams& SetOnAccepted(const FAction& handler) { OnAccepted = handler; return *this; }
    DescCustomDialogParams& FillWithText(const QString& text);
    DescCustomDialogParams& SetDefaultSpacing(bool defaultSpacing) { DefaultSpacing = defaultSpacing; return *this; }
    DescCustomDialogParams& AddButton(QDialogButtonBox::ButtonRole role, const QString& text, const FAction& action = []{})
    {
        Buttons.append(std::make_tuple(role, text, action));
        return *this;
    }
    DescCustomDialogParams& SetView(QWidget* view) { Q_ASSERT(View == nullptr); View = view; return *this; }
};

struct DescShowDialogParams
{
    bool Modal = true;
    bool ResizeToDefault = false;

    DescShowDialogParams& SetModal(bool modal) { Modal = modal; return *this; }
    DescShowDialogParams& SetResizeToDefault(bool resize) { ResizeToDefault = resize; return *this; }
};


struct DescTableViewParams
{
    struct ColumnParam
    {
        bool Visible = true;
        bool CanBeHidden = false;

        ColumnParam& SetVisible(bool visible) { Visible = visible; return *this; }
        ColumnParam& SetCanBeHidden(bool canBeHidden) { CanBeHidden = canBeHidden; return *this; }
        ColumnParam& HideColumn() { Visible = false; CanBeHidden = true; return *this;  }
        ColumnParam& RemoveColumn() { Visible = false; CanBeHidden = false; return *this;  }
    };

    DescTableViewParams(const QSet<qint32>& ignoreColumns);
    DescTableViewParams() : UseStandardActions(true) {}

    DescTableViewParams& SetUseStandardActions(bool use)
    {
        UseStandardActions = use;
        return *this;
    }

    DescTableViewParams& ShowColumns(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            SetColumnParam(column, ColumnParam().SetVisible(true));
        }
        return *this;
    }

    DescTableViewParams& HideColumns(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            SetColumnParam(column, ColumnParam().HideColumn());
        }
        return *this;
    }

    DescTableViewParams& AddColumns(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            SetColumnParam(column, ColumnParam().SetVisible(true).SetCanBeHidden(true));
        }
        return *this;
    }

    DescTableViewParams& RemoveColumns(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            SetColumnParam(column, ColumnParam().RemoveColumn());
        }
        return *this;
    }

    DescTableViewParams& RemoveTill(qint32 count)
    {
        while(--count != -1) {
            SetColumnParam(count, ColumnParam().RemoveColumn());
        }
        return *this;
    }


    DescTableViewParams& SetColumnParam(qint32 column, const ColumnParam& param) { ColumnsParams.insert(column, param); return *this; }
    DescTableViewParams& SetStateTag(const Latin1Name& stateTag) { StateTag = stateTag; return *this; }

    QHash<qint32, ColumnParam> ColumnsParams;
    Latin1Name StateTag;
    bool UseStandardActions;
};

inline DescTableViewParams::DescTableViewParams(const QSet<qint32>& ignoreColumns)
    : DescTableViewParams()
{
    for(const auto& column : ignoreColumns) {
        ColumnsParams.insert(column, ColumnParam());
    }
}

using WidgetsGlobalTableActionsScopeHandlersPtr = SharedPointer<struct WidgetsGlobalTableActionsScopeHandlers>;

#endif // WIDGETSDECLARATIONS_H
