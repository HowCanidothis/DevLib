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
        , FullParentSize(true)
    {}

    QuadTreeF::BoundingRect_Location Location;
    qint32 Delay;
    QWidget* RelativeParent;
    QPoint Offset;
    bool FullParentSize;

    DescWidgetsLocationAttachmentParams& SetOffset(const QPoint& offset) { Offset = offset; return *this; }
    DescWidgetsLocationAttachmentParams& SetDelay(qint32 delay) { Delay = delay; return *this; }
    DescWidgetsLocationAttachmentParams& SetRelativeParent(QWidget* parent) { RelativeParent = parent; return *this; }
    DescWidgetsLocationAttachmentParams& DisableFullParentSize() { FullParentSize = false; return *this; }
};

struct WidgetsDialogsManagerButtonStruct
{
    QDialogButtonBox::ButtonRole Role;
    FTranslationHandler Text;

    WidgetsDialogsManagerButtonStruct()
        : Role(QDialogButtonBox::RejectRole)
    {}
    WidgetsDialogsManagerButtonStruct(QDialogButtonBox::ButtonRole role, const FTranslationHandler& text)
        : Role(role)
        , Text(text)
    {}
};

enum class ButtonRole
{
    Action,
    Icon = 1,
    Save = 2,
    Reset = 3,
    Cancel = 4,
    AddIcon = 5,
    Add = 6,
    Tab = 7,
    FloatingButton = 9,
    IconWithText = 13
};

struct WidgetsDialogsManagerDefaultButtons
{
    static WidgetsDialogsManagerButtonStruct ConfirmButton();
    static WidgetsDialogsManagerButtonStruct CloseButton();
    static WidgetsDialogsManagerButtonStruct ApplyButton();
    static WidgetsDialogsManagerButtonStruct SaveButton();
    static WidgetsDialogsManagerButtonStruct OkButton();
    static WidgetsDialogsManagerButtonStruct CancelButton();
    static WidgetsDialogsManagerButtonStruct DiscardButton();
    static WidgetsDialogsManagerButtonStruct DeleteButton();
    static WidgetsDialogsManagerButtonStruct ReplaceButton();
    static WidgetsDialogsManagerButtonStruct MergeButton();
    static WidgetsDialogsManagerButtonStruct InsertButton();
    static WidgetsDialogsManagerButtonStruct SelectButton();

    static WidgetsDialogsManagerButtonStruct SaveRoleButton(const FTranslationHandler& text);
    static WidgetsDialogsManagerButtonStruct CancelRoleButton(const FTranslationHandler& text);
    static WidgetsDialogsManagerButtonStruct DiscardRoleButton(const FTranslationHandler& text);
    static WidgetsDialogsManagerButtonStruct ActionRoleButton(const FTranslationHandler& text);

    Q_DECLARE_TR_FUNCTIONS(WidgetsDialogsManagerDefaultButtons)
};

struct DescCustomDialogParams
{
    QVector<WidgetsDialogsManagerButtonStruct> Buttons;
    QWidget* View = nullptr;
    qint32 DefaultButtonIndex = 0;
    bool DefaultSpacing = true;
    FTranslationHandler Title;
    std::function<void (const QVector<QAbstractButton*>&)> OnInitialized;
    std::function<void (qint32)> OnDone;
    bool Resizeable = false;

    DescCustomDialogParams& SetResizeable(){ Resizeable = true; return *this; }
    DescCustomDialogParams& SetOnDone(const std::function<void (qint32)>& onDone){ OnDone = onDone; return *this; }
    DescCustomDialogParams& SetOnInitialized(const std::function<void (const QVector<QAbstractButton*>&)>& onInitialized) { OnInitialized = onInitialized; return *this; }
    DescCustomDialogParams& SetTitle(const FTranslationHandler& title) { Title = title; return *this; }
    DescCustomDialogParams& FillWithText(const QString& text);
    DescCustomDialogParams& SetDefaultSpacing(bool defaultSpacing) { DefaultSpacing = defaultSpacing; return *this; }
    DescCustomDialogParams& AddButton(const WidgetsDialogsManagerButtonStruct& button)
    {
        Buttons.append(button);
        return *this;
    }
    template<typename ... Args>
    DescCustomDialogParams& AddButtons(const Args&... buttons)
    {
        adapters::Combine([&](const auto& button){
            AddButton(button);
        }, buttons...);
        return *this;
    }
    DescCustomDialogParams& SetView(QWidget* view) { Q_ASSERT(View == nullptr); View = view; return *this; }
};
Q_DECLARE_METATYPE(DescCustomDialogParams)

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
    DescTableViewParams() {}

    DescTableViewParams& SetUseStandardActions(bool use)
    {
        UseStandardActions = use;
        return *this;
    }

    DescTableViewParams& CreateStandardActionHandlers()
    {
        UseStandardActionHandlers = true;
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
    bool UseStandardActions = true;
    bool UseMeasurementDelegates = false;
    bool UseStandardActionHandlers = false;

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
DECLARE_GLOBAL_CHAR(InvertedModel)
DECLARE_GLOBAL_CHAR(ActionWidget)
}

#endif // WIDGETSDECLARATIONS_H
