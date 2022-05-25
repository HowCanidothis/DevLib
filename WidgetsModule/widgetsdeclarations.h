#ifndef WIDGETSDECLARATIONS_H
#define WIDGETSDECLARATIONS_H

#include <QHash>

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

#endif // WIDGETSDECLARATIONS_H
