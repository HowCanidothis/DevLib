#ifndef WIDGETSDECLARATIONS_H
#define WIDGETSDECLARATIONS_H

#include <QHash>
#include <QDialogButtonBox>

#include <SharedModule/internal.hpp>

enum class GroupKeyboardSeparator {
    Space,
    Semicolon,
    Tab,
    Hash,
    Pipe,
    Comma,
    First = Space,
    Last = Comma,
};

template<>
struct EnumHelper<GroupKeyboardSeparator>
{
    static QStringList GetNames() { return { tr("Space"), ";", tr("Tab"), "#", "|", ","}; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

enum class DecimalKeyboardSeparator {
    Dot,
    Comma,
    First = Dot,
    Last = Comma,
};

template<>
struct EnumHelper<DecimalKeyboardSeparator>
{
    static QStringList GetNames() { return { ".", ","}; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

template<class T> class TViewModelsTableBase;
template<class T> class TViewModelsEditTable;

struct DescWidgetsLocationAttachmentParams
{
    DescWidgetsLocationAttachmentParams(QuadTreeF::BoundingRect_Location location = QuadTreeF::Location_Center)
        : Location(location)
        , Delay(0)
        , RelativeParent(nullptr)
    {}

    QuadTreeF::BoundingRect_Location Location;
    qint32 Delay;
    QWidget* RelativeParent;
    QPoint Offset;

    DescWidgetsLocationAttachmentParams& SetOffset(const QPoint& offset) { Offset = offset; return *this; }
    DescWidgetsLocationAttachmentParams& SetDelay(qint32 delay) { Delay = delay; return *this; }
    DescWidgetsLocationAttachmentParams& SetRelativeParent(QWidget* parent) { RelativeParent = parent; return *this; }
};

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
        bool CanBeHidden = true;
        qint32 ReplacePlaceTo = -1;

        ColumnParam& MoveTo(qint32 logicalIndex) { ReplacePlaceTo = logicalIndex; return *this; }
        ColumnParam& ShowAlways() { Visible = true; CanBeHidden = false; return *this; }
        ColumnParam& Show() { Visible = true; return *this; }
        ColumnParam& Add() { Visible = true; CanBeHidden = true; return *this;  }
        ColumnParam& Hide() { Visible = false; return *this;  }
        ColumnParam& Remove() { Visible = false; CanBeHidden = false; return *this;  }
    };

    DescTableViewParams(const QSet<qint32>& ignoreColumns);
    DescTableViewParams() : UseStandardActions(true), UseMeasurementDelegates(false) {}

    DescTableViewParams& SetUseStandardActions(bool use)
    {
        UseStandardActions = use;
        return *this;
    }

    DescTableViewParams& ShowColumns(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            Column(column).Show();
        }
        return *this;
    }

    DescTableViewParams& ShowColumnsAlways(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            Column(column).ShowAlways();
        }
        return *this;
    }

    DescTableViewParams& HideColumns(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            Column(column).Hide();
        }
        return *this;
    }

    DescTableViewParams& AddColumns(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            Column(column).Add();
        }
        return *this;
    }

    DescTableViewParams& RemoveColumns(const QVector<qint32>& columns)
    {
        for(auto column : columns) {
            Column(column).Remove();
        }
        return *this;
    }

    DescTableViewParams& RemoveTill(qint32 count)
    {
        while(--count != -1) {
            Column(count).Remove();
        }
        return *this;
    }

    DescTableViewParams& HideTill(qint32 count)
    {
        while(--count != -1) {
            Column(count).Hide();
        }
        return *this;
    }

    DescTableViewParams& Make(const std::function<void (DescTableViewParams& params)>& handler)
    {
        handler(*this);
        return *this;
    }

    ColumnParam& Column(qint32 column)
    {
        auto foundIt = ColumnsParams.find(column);
        if(foundIt == ColumnsParams.end()) {
            return *ColumnsParams.insert(column, ColumnParam());
        }
        return foundIt.value();
    }
    DescTableViewParams& SetStateTag(const Latin1Name& stateTag) { StateTag = stateTag; return *this; }

    DescTableViewParams& SetUseMeasurementDelegates(bool use)
    {
        UseMeasurementDelegates = use;
        return *this;
    }

    QHash<qint32, ColumnParam> ColumnsParams;
    Latin1Name StateTag;
    bool UseStandardActions;
    bool UseMeasurementDelegates;

    bool StretchLastSection = true;
    bool SectionsMovable = true;
    bool SectionsClickable = true;
    bool HighlightSections = true;
    bool DropIndicatorShown = true;
    bool SortIndicatorShown = true;
    DescTableViewParams& SetStretchLastSection (bool state) { StretchLastSection = state; return *this; }
    DescTableViewParams& SetSectionsMovable    (bool state) { SectionsMovable = state; return *this; }
    DescTableViewParams& SetSectionsClickable  (bool state) { SectionsClickable = state; return *this; }
    DescTableViewParams& SetHighlightSections  (bool state) { HighlightSections = state; return *this; }
    DescTableViewParams& SetDropIndicatorShown (bool state) { DropIndicatorShown = state; return *this; }
    DescTableViewParams& SetSortIndicatorShown (bool state) { SortIndicatorShown = state; return *this; }
};

inline DescTableViewParams::DescTableViewParams(const QSet<qint32>& ignoreColumns)
    : DescTableViewParams()
{
    for(const auto& column : ignoreColumns) {
        ColumnsParams.insert(column, ColumnParam().ShowAlways());
    }
}

using WidgetsGlobalTableActionsScopeHandlersPtr = SharedPointer<struct WidgetsGlobalTableActionsScopeHandlers>;
template<class Enum>
using ViewModelsCategoriesContainer = QVector<std::pair<FTranslationHandler, QVector<Enum>>>;
template<class Enum>
ContainerBuilder<ViewModelsCategoriesContainer<Enum>> ViewModelsCategoriesContainerCreate() { return ContainerBuilder<ViewModelsCategoriesContainer<Enum>>(); }
using ModelsStandardListModelContainer = QVector<QHash<qint32, QVariant>>;

namespace WidgetProperties {
DECLARE_GLOBAL_CHAR(ExtraFieldsCount)
DECLARE_GLOBAL_CHAR(InnerSplitter)
DECLARE_GLOBAL_CHAR(Footer)
}

#endif // WIDGETSDECLARATIONS_H
