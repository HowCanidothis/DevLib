#include "notifyconsole.h"
#include "ui_notifyconsole.h"

#include <QStyledItemDelegate>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QSortFilterProxyModel>

#include "WidgetsModule/Utils/iconsmanager.h"
#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Delegates/delegates.h"
#include "WidgetsModule/Models/viewmodeltreebase.h"
#include "WidgetsModule/Models/modelsfiltermodelbase.h"

#include "notifywidget.h"
#include "notifymanager.h"

class RichTextItemDelegate : public QStyledItemDelegateBase
{
    using Super = QStyledItemDelegateBase;
public:
    using Super::Super;

    void paint(QPainter *painter, const QStyleOptionViewItem &inOption,
                                     const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &inOption, const QModelIndex &index) const override;

    qint32 Width = 0;
    qint32 Height = 40;
};

void RichTextItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem& inOption,
                                 const QModelIndex &index) const {
    QStyleOptionViewItem option = inOption;
    initStyleOption(&option, index);

    QStyle* style = option.widget? option.widget->style() : QApplication::style();

    QTextOption textOption;
    textOption.setAlignment(inOption.displayAlignment);
    textOption.setWrapMode(QTextOption::WordWrap);

    QTextDocument doc;
    doc.setDefaultFont(option.font);
    doc.setDefaultTextOption(textOption);
    doc.setTextWidth(option.rect.width());
    doc.setPageSize(option.rect.size());
    doc.setIndentWidth(0.0);
    doc.setDocumentMargin(0);
    doc.setHtml(option.text);

    /// Painting item without text
    option.text = QString();
    option.state &= ~QStyle::State_HasFocus;
    style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

    QAbstractTextDocumentLayout::PaintContext ctx;

    ctx.palette = option.palette;

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &option);
    painter->save();
    painter->translate(textRect.topLeft());
    if(textOption.alignment() & Qt::AlignVCenter) {
        auto h = doc.size().height();
        if(h < textRect.height()) {
            painter->translate(0, (textRect.height() - h) / 2);
        }
    }
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize RichTextItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    return QSize(Width, Height);
}

class NotifyTreeNode : public ModelsTreeItemBase
{
    using Super = ModelsTreeItemBase;
public:
    enum Type {
        T_Root,
        T_Message
    };

    NotifyTreeNode(const NotifyConsoleDataPtr& data)
        : m_data(data)
        , m_watched(false)
    {}

    virtual QString GetLabel() const { return QString::number(GetRow()); }
    virtual qint32 GetType() const { return NotifyTreeNode::T_Message; }

    NotifyConsoleDataPtr& GetData() { return m_data; }
    bool IsWatched() const { return m_watched; }
    void SetIsWatched(bool watched) { m_watched = watched; }

private:
    NotifyConsoleDataPtr m_data;
    bool m_watched;
};

class NotifyTreeFolderNode : public ModelsTreeItemBase
{
    using Super = ModelsTreeItemBase;
public:
    NotifyTreeFolderNode()
    {}
    NotifyTreeFolderNode(const Name& id, const TranslatedStringPtr& text)
        : m_text(text)
        , m_id(id)
    {}

    virtual QString GetLabel() const { return *m_text; }
    virtual qint32 GetType() const { return NotifyTreeNode::T_Root; }

    const TranslatedString& GetText() { return *m_text; }
    const Name& GetId() const { return m_id; }

private:
    TranslatedStringPtr m_text;
    Name m_id;
};

Q_DECLARE_METATYPE(ModelsTreeItemBase*);

class NotifyConsoleViewModel : public TViewModelTreeBase<ModelsTree>
{
    using Super = TViewModelTreeBase<ModelsTree>;
public:
    enum Columns {
        C_CheckBox,
        C_Icon,
        C_Time = C_Icon,
        C_Description,
        C_Count
    };

    enum Roles {
        R_Folder = Qt::UserRole,
        R_Message,
        R_Watched
    };
    
    NotifyConsoleViewModel(QObject* parent)
        : Super(parent)
    {
//        m_roleHorizontalHeaderDataHandlers.insert(Qt::DisplayRole, [](qint32 column) -> QVariant {
//            switch(column) {
//            case C_CheckBox: return tr("Issue Visibility");
//            case C_Time: return tr("Issue Time");
//            case C_Description: return tr("Issue Description");
//            default: break;
//            }
//            return QVariant();
//        });
    }

    // QAbstractItemModel interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override 
    {
        if(!index.isValid()){
            return false;
        }

        auto* item = AsItem(index);
        if(item->GetType() != NotifyTreeNode::T_Message) {
            return false;
        }

        if(role == Qt::EditRole && index.column() == C_CheckBox) {
            GetData()->Edit(item, [&]{
                *item->As<NotifyTreeNode>()->GetData()->Data->Visible = value.toBool();
            }, {Qt::DecorationRole});
            return true;
        }
        if(role == R_Watched) {
            GetData()->Edit(item, [&]{
                item->As<NotifyTreeNode>()->SetIsWatched(value.toBool());
            }, {R_Watched});
        }
        return false;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        auto* item = AsItem(index);
        auto selectable = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        if(item->GetType() == NotifyTreeNode::T_Message) {
            if(index.column() == C_CheckBox && item->As<NotifyTreeNode>()->GetData()->Data->Visible != nullptr) {
                return selectable | Qt::ItemIsEditable;
            }
            return selectable;
        }
        return selectable;
    }
    
    int columnCount(const QModelIndex&) const override
    {
        return C_Count;
    }
    QVariant data(const QModelIndex& index, int role) const override
    {
        if(!index.isValid()) {
            return QVariant();
        }

        auto* item = AsItem(index);
        if(item->GetType() == -1) {
            return QVariant();
        }
        if(item->GetType() == NotifyTreeNode::T_Root) {
            switch(role) {
            case R_Watched:
                return false;
            case Qt::DisplayRole:
                if(index.column() == C_Description) {
                    return item->As<NotifyTreeFolderNode>()->GetLabel();
                }
                break;
            case R_Folder:
                return QVariant::fromValue(item);
            default: break;
            }
            return QVariant();
        }
        const auto& data = item->As<NotifyTreeNode>()->GetData()->Data;

        switch (role) {
        case R_Message:
            return QVariant::fromValue(item);
        case R_Watched:
            return item->As<NotifyTreeNode>()->IsWatched();
        case Qt::EditRole: {
            switch (index.column()) {
            case C_CheckBox: return data->Visible != nullptr ? data->Visible->Native() : QVariant();
            default: break;
            }
            return QVariant();
        }
        case Qt::DisplayRole: {
            switch (index.column()) {
            case C_Time: return LanguageSettings::DateTimeToString(data->DateTime);
            case C_Description: return data->Body;
            default: return QVariant();
            }
        }
        case Qt::DecorationRole:
            if(index.column() == C_Icon) {
                switch(data->Type) {
                case NotifyManager::Warning: return m_warningIcon;
                case NotifyManager::Error: return m_errorIcon;
                case NotifyManager::Info: return m_infoIcon;
                default: break;
                }
            }
            break;
        case Qt::TextAlignmentRole:
            if(index.column() == C_Time) {
                return Qt::AlignCenter;
            }
            break;
        default:
            break;
        }

        return QVariant();
    }
};

class WatchedDelegate : public QStyledItemDelegateBase
{
    using Super = QStyledItemDelegateBase;
public:
    using Super::Super;

    void paint(QPainter *painter, const QStyleOptionViewItem& inOption,
                                 const QModelIndex &index) const
    {
        Super::paint(painter,inOption,index);
        bool watched = false;
        auto childCount = index.model()->rowCount(index);
        for(qint32 i(0); i < childCount; ++i) {
            if(index.model()->index(i,0,index).data(NotifyConsoleViewModel::R_Watched).toBool()) {
                watched = true;
                break;
            }
        }
        if(!watched) {
            watched = index.data(NotifyConsoleViewModel::R_Watched).toBool();
        }
        if(!watched) {
            const double size = 2.0;
            auto h = inOption.rect.height();
            painter->setPen(Qt::NoPen);
            painter->setBrush(SharedSettings::GetInstance().StyleSettings.DefaultBlueColor.Native());
            painter->drawEllipse(QPointF(inOption.rect.topRight() + QPoint(-9 -size, (h - size) / 2)),size,size);
        }
    }
};

class CollapseExpandItemDelegate : public DelegatesCheckBox
{
    using Super = DelegatesCheckBox;
public:
    using Super::Super;

    void paint(QPainter *painter, const QStyleOptionViewItem &inOption,
                                     const QModelIndex &index) const override
    {
        if(!index.data(NotifyConsoleViewModel::R_Message).isNull()) {
            Super::paint(painter,inOption, index);
            return;
        }
        auto* treeView = reinterpret_cast<const WidgetsTreeViewBase*>(inOption.widget);
        IconsSvgIcon icon;
        if(treeView->isExpanded(index)) {
            icon = IconsManager::GetInstance().GetIcon("CollapseIcon");
        } else {
            icon = IconsManager::GetInstance().GetIcon("ExpandIcon");
        }
        QStyledItemDelegate::paint(painter,inOption, index);
        icon.paint(painter, inOption.rect.adjusted(9,9,-9,-9));
    }
};

class NotifyConsoleSortFilterViewModel : public ViewModelsFilterModelBase
{
    using Super = ViewModelsFilterModelBase;
public:
    NotifyConsoleSortFilterViewModel(QObject* parent)
        : Super(parent)
        , SeverityFilter(NotifyManager::Error | NotifyManager::Warning)
    {
        auto invalidate = [this]{
            InvalidateFilter();
        };
        StringFilter.Subscribe(invalidate);
        SeverityFilter.Subscribe(invalidate);
        FilterHandler = [this](qint32 source_row, const QModelIndex& parent)
        {
            auto* parentItem = getConsoleViewModel()->AsItem(parent);
            if(parentItem->GetType() == -1) {
                return true;
            }

            const auto& data = parentItem->GetChilds().at(source_row)->As<NotifyTreeNode>()->GetData()->Data;
            if(!(data->Type & SeverityFilter)) {
                return false;
            }
            if(!StringFilter.Native().isEmpty()) {
                return data->Body.contains(StringFilter, Qt::CaseInsensitive) || parentItem->GetLabel().contains(StringFilter, Qt::CaseInsensitive);
            }
            return true;
        };
    }

    LocalPropertyString StringFilter;
    LocalPropertyInt SeverityFilter;
    // QSortFilterProxyModel interface
protected:
    NotifyConsoleViewModel* getConsoleViewModel() const { return reinterpret_cast<NotifyConsoleViewModel*>(sourceModel()); }


    QVariant data(const QModelIndex& index, qint32 role) const override
    {
        if(index.column() != NotifyConsoleViewModel::C_Time || parent(index).isValid()) {
            return Super::data(index, role);
        }
        if(role == Qt::DisplayRole) {
            return rowCount(index);
        }
        return Super::data(index, role);
    }
    QVariant headerData(qint32 section, Qt::Orientation orientation, qint32 role) const override
    {
        if(orientation == Qt::Vertical && role == Qt::DisplayRole) {
            return section + 1;
        }
        return Super::headerData(section, orientation, role);
    }
};

NotifyConsole::NotifyConsole(QWidget *parent)
    : QWidget(parent)
    , IsOpened(true)
    , IsShowWarnings(true)
    , IsShowInfos(false)
    , IsShowErrors(true)
    , ui(new Ui::NotifyConsole)
    , m_updateErrors(500)
{
    ui->setupUi(this);

    ui->IssuesView->setUniformRowHeights(true);

    ui->BtnShowCategories->setVisible(false);

    m_model = ::make_shared<ModelsTree>();
    m_model->SetRoot(::make_shared<ModelsTreeItemBase>());

    auto* viewModel = new NotifyConsoleViewModel(this);
    viewModel->SetData(m_model);
    auto* filterModel = m_filterViewModel = new NotifyConsoleSortFilterViewModel(this);
    filterModel->Invalidate.ConnectFrom(CDL, m_model->OnRowsInserted, m_model->OnRowsRemoved);
    filterModel->setDynamicSortFilter(false);
    filterModel->setSourceModel(viewModel);

    auto* filterNoneViewModel = new ViewModelsFilterModelBase(this);
    filterNoneViewModel->setDynamicSortFilter(false);
    filterNoneViewModel->Invalidate.ConnectFrom(CDL, filterModel->OnInvalidated);
    filterNoneViewModel->FilterHandler = [filterModel](qint32 r, const QModelIndex& parent) {
        if(!parent.isValid()) {
            auto count = filterModel->rowCount(filterModel->index(r,0));
            return count > 0;
        }
        return true;
    };
    filterNoneViewModel->GetDataHandler = [filterNoneViewModel](const QModelIndex& index, qint32 role) -> QVariant {
        if(role == Qt::BackgroundRole) {
            auto parentIndex = index.parent();
            bool odd;
            if(parentIndex.isValid()) {
                odd = parentIndex.row() % 2;
            } else {
                odd = index.row() % 2;
            }
            return odd ? SharedSettings::GetInstance().StyleSettings.DisabledTableCellColor.Native() : SharedSettings::GetInstance().StyleSettings.DisabledTableCellAltColor.Native();
        }
        return filterNoneViewModel->DefaultGetData(index, role);
    };
    filterNoneViewModel->setSourceModel(filterModel);
    ui->IssuesView->setModel(filterNoneViewModel);

//    auto viewModel = new NotifyConsoleViewModel(this);
//    viewModel->SetData(Data);

//    auto* filterModel = new ConsoleSortFilterViewModel(this);
//    filterModel->setSourceModel(viewModel);
//    ui->IssuesView->setModel(filterModel);

//    WidgetTableViewWrapper(ui->TableIssues).InitializeHorizontal()->hide();
//    auto handlers = WidgetsGlobalTableActionsScope::AddDefaultHandlers(ui->TableIssues);
//    handlers->IsReadOnly = true;
//    handlers->ShowAll();

    ui->IssuesView->setItemDelegateForColumn(NotifyConsoleViewModel::C_CheckBox, new CollapseExpandItemDelegate(ui->IssuesView));
    ui->IssuesView->setItemDelegateForColumn(NotifyConsoleViewModel::C_Description, new RichTextItemDelegate(ui->IssuesView));
    ui->IssuesView->setItemDelegateForColumn(NotifyConsoleViewModel::C_Time, new WatchedDelegate(ui->IssuesView));
    ui->IssuesView->setRootIsDecorated(false);
    ui->IssuesView->header()->hide();
    ui->IssuesView->header()->resizeSection(NotifyConsoleViewModel::C_CheckBox, 24);
    ui->IssuesView->header()->resizeSection(NotifyConsoleViewModel::C_Icon, 24);
    ui->IssuesView->header()->resizeSection(NotifyConsoleViewModel::C_Time, 200);
    IsOpened.Subscribe([this]{
        setVisible(IsOpened);
    });

    WidgetPushButtonWrapper(ui->BtnCloseMinor).SetOnClicked([this]{
        IsOpened = false;
    });
    m_connectors.AddConnector<LocalPropertiesPushButtonConnector>(&IsShowErrors, ui->BtnShowErrors);
    m_connectors.AddConnector<LocalPropertiesPushButtonConnector>(&IsShowInfos, ui->BtnShowInfo);
    m_connectors.AddConnector<LocalPropertiesPushButtonConnector>(&IsShowWarnings, ui->BtnShowWarnings);
    m_connectors.AddConnector<LocalPropertiesLineEditConnector>(&filterModel->StringFilter, ui->Filter);

    auto changeFilter = [filterModel](const LocalPropertyBool& show, NotifyManager::MessageType type){
        auto current = filterModel->SeverityFilter.Native();
        if(show) {
            current |= type;
        } else {
            current &= ~type;
        }
        filterModel->SeverityFilter = current;
    };

    IsShowWarnings.Subscribe([changeFilter, this]{ changeFilter(IsShowWarnings, NotifyManager::Warning); });
    IsShowInfos.Subscribe([changeFilter, this]{ changeFilter(IsShowInfos, NotifyManager::Info); });
    IsShowErrors.Subscribe([changeFilter, this]{ changeFilter(IsShowErrors, NotifyManager::Error); });

    connect(ui->IssuesView, &QTreeView::doubleClicked, [](const QModelIndex& current) {
        if(current.column() == NotifyConsoleViewModel::C_CheckBox) {
            return;
        }
        auto* item = current.data(NotifyConsoleViewModel::R_Message).value<ModelsTreeItemBase*>();
        if(item == nullptr) {
            return;
        }
        const auto& data = item->As<NotifyTreeNode>()->GetData();
        if(data->ErrorHandler != nullptr && data->ErrorHandler->Action != nullptr) {
            data->ErrorHandler->Action();
        }
        if(data->WarningHandler != nullptr && data->WarningHandler->Action != nullptr) {
            data->WarningHandler->Action();
        }
    });

    connect(ui->IssuesView, &QTreeView::clicked, [this](const QModelIndex& mi) {
        auto* item = mi.data(NotifyConsoleViewModel::R_Folder).value<ModelsTreeItemBase*>();
        if(item == nullptr) {
            return;
        }

        bool expanded = ui->IssuesView->isExpanded(mi.model()->index(mi.row(),0,mi.parent()));
        ViewModelWrapper(ui->IssuesView->model()).ForeachModelIndex(mi.parent(), [this](const QModelIndex& pmi) {
            if(ui->IssuesView->isExpanded(pmi)) {
                ViewModelWrapper(ui->IssuesView->model()).ForeachModelIndex(pmi, [&](const QModelIndex& index) {
                    ui->IssuesView->model()->setData(index, true, NotifyConsoleViewModel::R_Watched);
                    return false;
                },0,false);
                ui->IssuesView->collapse(pmi);
            }
            return false;
        }, 0, false);
        if(!expanded) {
            ui->IssuesView->expand(mi);
        }
    });

//    m_updateErrors.Connect(CONNECTION_DEBUG_LOCATION, [this]{
//        Data->UpdateUi([]{});
//    });

    auto emitCountChanged = [this, filterNoneViewModel]{
        OnShownMessagesCountChanged(filterNoneViewModel->rowCount());
    };
    connect(filterNoneViewModel, &QSortFilterProxyModel::rowsInserted, emitCountChanged);
    connect(filterNoneViewModel, &QSortFilterProxyModel::rowsRemoved, emitCountChanged);
    connect(filterNoneViewModel, &QSortFilterProxyModel::modelReset, emitCountChanged);
}

void NotifyConsole::AddWarning(const QString& data, const FAction& handler)
{
    auto consoleData = ::make_shared<NotifyConsoleData>();
    consoleData->Data = ::make_shared<NotifyData>(NotifyManager::Warning, data);
    consoleData->WarningHandler = ::make_scoped<NotifyWarningData>(handler);

    Add(consoleData);
}

void NotifyConsole::Add(const NotifyConsoleDataPtr& data)
{
    m_model->Insert(0, ::make_shared<NotifyTreeNode>(data), getOrCreateFolder(Name()));
}

NotifyConsole::~NotifyConsole()
{
    delete ui;
}

void NotifyConsole::SetVisibility(ElementVisibilityFlags visibility)
{
    ui->BtnCloseMinor->setVisible(visibility.TestFlag(ElementVisibility_Close));
    ui->BtnClear->setVisible(visibility.TestFlag(ElementVisibility_Clear));
    ui->BtnShowWarnings->setVisible(visibility.TestFlag(ElementVisibility_ShowWarnings));
    ui->Filter->setVisible(visibility.TestFlag(ElementVisibility_Filter));
    ui->BtnShowInfo->setVisible(visibility.TestFlag(ElementVisibility_ShowInfos));
    ui->BtnShowErrors->setVisible(visibility.TestFlag(ElementVisibility_ShowErrors));
}

void NotifyConsole::SetErrorIcon(const IconsSvgIcon& icon)
{
    ui->BtnShowErrors->setIcon(icon);
}

void NotifyConsole::SetCleanIcon(const IconsSvgIcon& icon)
{
    ui->BtnClear->setIcon(icon);
}

void NotifyConsole::SetWarningIcon(const IconsSvgIcon& icon)
{
    ui->BtnShowWarnings->setIcon(icon);
}

void NotifyConsole::SetInfoIcon(const IconsSvgIcon& icon)
{
    ui->BtnShowInfo->setIcon(icon);
}

void NotifyConsole::AttachErrorsContainer(const Name& folderId, LocalPropertyErrorsContainer* container, const std::function<void (const Name&)>& handler, const TranslatedStringPtr& folderText)
{
    Q_ASSERT(!m_permanentErrors.contains(container));

    auto addError = [this, handler, container, folderId, folderText](const LocalPropertyErrorsContainerValue& error){
        auto id = error.Id;
        auto consoleData = ::make_shared<NotifyConsoleData>();
        consoleData->ErrorHandler = new NotifyErrorContainerData( [handler, id]{ handler(id); }, container, id );
        switch(error.Type) {
        case QtMsgType::QtCriticalMsg:
        case QtMsgType::QtFatalMsg:
            consoleData->Data = ::make_shared<NotifyData>(NotifyManager::Error, error.Error->Native(), error.Visible);
            break;
        default:
            consoleData->Data = ::make_shared<NotifyData>(NotifyManager::Warning, error.Error->Native(), error.Visible);
            break;
        }

        auto* pConsoleData = consoleData.get();
        auto* pError = error.Error.get();
        error.Error->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this, pError, pConsoleData]{
            pConsoleData->Data->Body = pError->Native();
            m_updateErrors();
        }).MakeSafe(pConsoleData->ErrorHandler->Connections);
//        if(pConsoleData->Data->Visible != nullptr){
//            pConsoleData->Data->Visible->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, []{
                //update check role;
//                Edit
//            }).MakeSafe(pConsoleData->ErrorHandler->Connections);
//        }
        m_model->Insert(0, ::make_shared<NotifyTreeNode>(consoleData), getOrCreateFolder(folderId, folderText));
    };

    auto removeError = [this, handler, container](const LocalPropertyErrorsContainerValue& error) {
        m_permanentErrorsToErase[container].insert(error.Id);
        erasePermanentErrors();
    };

    for(const auto& error : *container) {
        addError(error);
    }

    container->OnErrorAdded.Connect(CONNECTION_DEBUG_LOCATION, addError).MakeSafe(m_permanentErrors[container]);
    container->OnErrorRemoved.Connect(CONNECTION_DEBUG_LOCATION, removeError).MakeSafe(m_permanentErrors[container]);
}

void NotifyConsole::erasePermanentErrors()
{
    m_erasePermanentErrors.Call(CONNECTION_DEBUG_LOCATION,  [this]{
        if(m_permanentErrorsToErase.isEmpty()) {
            return;
        }

        QSet<ModelsTreeItemBase*> itemsToRemove;
        for(const auto* folder : m_folders) {
            for(const auto& item : folder->GetChilds()) {
                const auto& data = item->As<NotifyTreeNode>()->GetData();
                if(data->ErrorHandler != nullptr) {
                    auto foundIt = m_permanentErrorsToErase.find(data->ErrorHandler->Container);
                    if(foundIt != m_permanentErrorsToErase.end()) {
                        if(foundIt.value().contains(data->ErrorHandler->Id)) {
                            itemsToRemove.insert(item.get());
                        }
                    }
                }
            }
        }
        if(!itemsToRemove.isEmpty()) {
            m_model->Change([&]{
                for(auto* item : itemsToRemove) {
                    auto* folder = item->GetParent();
                    folder->RemoveChild(item->GetRow());
                    if(folder->GetChilds().isEmpty()) {
                        removeFolder(folder->As<NotifyTreeFolderNode>()->GetId());
                    }
                }
            });
        }
        m_permanentErrorsToErase.clear();
    });
}

NotifyTreeFolderNode* NotifyConsole::getOrCreateFolder(const Name& folderId, const TranslatedStringPtr& label)
{
    auto foundIt = m_folders.constFind(folderId);
    NotifyTreeFolderNode* folder;
    if(foundIt == m_folders.cend()) {
        auto newFolder = ::make_shared<NotifyTreeFolderNode>(folderId, folderId.IsNull() ? TRS(tr("Notifications")) : label);
        auto location = folderId.IsNull() ? 0 : 1;
        if(m_model->IsInScope()) {
            m_model->GetRoot()->InsertChild(location, newFolder);
        } else {
            m_model->Insert(location, newFolder, m_model->GetRoot());
        }
        folder = newFolder.get();
        m_folders.insert(folderId, folder);
    } else {
        folder = foundIt.value();
    }
    return folder;
}

void NotifyConsole::removeFolder(const Name& folderId)
{
    auto foundIt = m_folders.find(folderId);
    if(foundIt != m_folders.end()) {
        if(m_model->IsInScope()) {
            foundIt.value()->GetParent()->RemoveChild(foundIt.value()->GetRow());
        } else {
            m_model->Remove(foundIt.value());
        }
        m_folders.erase(foundIt);
    }
}

void NotifyConsole::DetachErrorsContainer(LocalPropertyErrorsContainer* container)
{
    if(!m_permanentErrors.contains(container)) {
        return;
    }
    for(const auto& error : *container) {
        m_permanentErrorsToErase[container].insert(error.Id);
    }

    m_permanentErrors.remove(container);
    erasePermanentErrors();
}

void NotifyConsole::on_BtnCloseConsole_clicked()
{
    IsOpened = false;
}

void NotifyConsole::on_BtnClear_clicked()
{
    QSet<ModelsTreeItemBase*> itemsToRemove;
    for(const auto* folder : m_folders) {
        for(const auto& item : folder->GetChilds()) {
            const auto& data = item->As<NotifyTreeNode>()->GetData();
            if(data->ErrorHandler == nullptr) {
                itemsToRemove.insert(item.get());
            }
        }
    }
    if(!itemsToRemove.isEmpty()) {
        m_model->Change([&]{
            for(auto* item : itemsToRemove) {
                auto* folder = item->GetParent();
                folder->RemoveChild(item->GetRow());
                if(folder->GetChilds().isEmpty()) {
                    removeFolder(folder->As<NotifyTreeFolderNode>()->GetId());
                }
            }
        });
    }
}
