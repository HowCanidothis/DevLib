#include "notifyconsole.h"
#include "ui_notifyconsole.h"

#include <QStyledItemDelegate>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QSortFilterProxyModel>

#include "WidgetsModule/Utils/iconsmanager.h"
#include "WidgetsModule/Delegates/delegates.h"

#include "notifywidget.h"
#include "notifymanager.h"

class RichTextItemDelegate : public QStyledItemDelegate
{
    using Super = QStyledItemDelegate;
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
    textOption.setAlignment(option.displayAlignment);

    QTextDocument doc;
    option.font.setPixelSize(option.font.pixelSize() + 1);
    doc.setDefaultFont(option.font);
    doc.setDefaultTextOption(textOption);
    doc.setTextWidth(option.rect.width());
    doc.setIndentWidth(0.0);
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
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize RichTextItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    return QSize(Width, Height);
}

class NotifyConsoleViewModel : public TViewModelsTableBase<NotifyConsoleDataWrapper>
{
    using Super = TViewModelsTableBase<NotifyConsoleDataWrapper>;
public:
    enum Columns {
        C_CheckBox,
        C_Icon,
        C_Time = C_Icon,
        C_Description,
        C_Count
    };
    
    NotifyConsoleViewModel(QObject* parent)
        : Super(parent)
    {
    }

    // QAbstractItemModel interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override 
    {
        if(!index.isValid()){
            return false;
        }
        if(role == Qt::EditRole && index.column() == C_CheckBox && GetData()->At(index.row())->Data->Visible != nullptr){
            GetData()->Edit(index.row(), [value](NotifyConsoleDataPtr& data){
                *data->Data->Visible = value.toBool();
            }, {C_CheckBox});
            return true;
        }
        return false;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if(index.column() == C_CheckBox && GetData()->At(index.row())->Data->Visible != nullptr) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        } else {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        }
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

        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole: {
            const auto& data = GetData()->At(index.row())->Data;
            switch (index.column()) {
            case C_Time: return data->DateTime;
            case C_CheckBox: return data->Visible != nullptr ? data->Visible->Native() : QVariant();
            case C_Description: return data->Body;
            default: return QVariant();
            }
        }
        case Qt::DecorationRole:
            if(index.column() == C_Icon) {
                switch(GetData()->At(index.row())->Data->Type) {
                case NotifyManager::Warning: return m_iconsContext.WarningIcon;
                case NotifyManager::Error: return m_iconsContext.ErrorIcon;
                case NotifyManager::Info: return m_iconsContext.InfoIcon;
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

class ConsoleSortFilterViewModel : public QSortFilterProxyModel
{
    using Super = QSortFilterProxyModel;
public:
    ConsoleSortFilterViewModel(QObject* parent)
        : Super(parent)
        , SeverityFilter(NotifyManager::Error | NotifyManager::Warning)
    {
        auto invalidate = [this]{
            m_invalidate.Call([this]{
                invalidateFilter();
            });
        };
        StringFilter.Subscribe(invalidate);
        SeverityFilter.Subscribe(invalidate);
    }

    LocalPropertyString StringFilter;
    LocalPropertyInt SeverityFilter;
    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    QVariant headerData(qint32 section, Qt::Orientation orientation, qint32 role) const override
    {
        if(orientation == Qt::Vertical && role == Qt::DisplayRole) {
            return section + 1;
        }
        return Super::headerData(section, orientation, role);
    }

private:
    NotifyConsoleViewModel* consoleModel() const { return reinterpret_cast<NotifyConsoleViewModel*>(sourceModel()); }

private:
    DelayedCallObject m_invalidate;
};

bool ConsoleSortFilterViewModel::filterAcceptsRow(int source_row, const QModelIndex&) const
{
    const auto& data = consoleModel()->GetData()->At(source_row);
    if(!(data->Data->Type & SeverityFilter)) {
        return false;
    }
    if(!StringFilter.Native().isEmpty()) {
        return data->Data->Body.contains(StringFilter, Qt::CaseInsensitive);
    }
    return true;
}

NotifyConsole::NotifyConsole(QWidget *parent)
    : QWidget(parent)
    , Data(::make_shared<NotifyConsoleDataWrapper>())
    , IsOpened(true)
    , IsShowWarnings(true)
    , IsShowInfos(false)
    , IsShowErrors(true)
    , OnShownMessagesCountChanged(500)
    , ui(new Ui::NotifyConsole)
    , m_updateErrors(500)
{
    ui->setupUi(this);

    ui->BtnShowCategories->setVisible(false);
    auto viewModel = new NotifyConsoleViewModel(this);
    viewModel->SetData(Data);

    auto* filterModel = new ConsoleSortFilterViewModel(this);
    filterModel->setSourceModel(viewModel);
    ui->TableIssues->setModel(filterModel);

    ui->TableIssues->setItemDelegateForColumn(NotifyConsoleViewModel::C_CheckBox, new DelegatesCheckBox(ui->TableIssues));
    ui->TableIssues->setItemDelegateForColumn(NotifyConsoleViewModel::C_Description, new RichTextItemDelegate(ui->TableIssues));
    ui->TableIssues->horizontalHeader()->resizeSection(0, 24);
    
    IsOpened.Subscribe([this]{
        setVisible(IsOpened);
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

    connect(ui->TableIssues, &QTableView::doubleClicked, [filterModel, this](const QModelIndex& current) {
        auto sourceIndex = filterModel->mapToSource(current);
        const auto& data = Data->At(sourceIndex.row());
        if(data->ErrorHandler != nullptr && data->ErrorHandler->Action != nullptr) {
            data->ErrorHandler->Action();
        }
        if(data->WarningHandler != nullptr && data->WarningHandler->Action != nullptr) {
            data->WarningHandler->Action();
        }
    });

    m_updateErrors.Connect(this, [this]{
        Data->UpdateUi([]{});
    });

    auto emitCountChanged = [this, filterModel]{
        OnShownMessagesCountChanged(filterModel->rowCount());
    };
    connect(filterModel, &QSortFilterProxyModel::rowsInserted, emitCountChanged);
    connect(filterModel, &QSortFilterProxyModel::rowsRemoved, emitCountChanged);
    connect(filterModel, &QSortFilterProxyModel::modelReset, emitCountChanged);
}

void NotifyConsole::AddWarning(const QString& data, const FAction& handler)
{
    auto consoleData = ::make_shared<NotifyConsoleData>();
    consoleData->Data = ::make_shared<NotifyData>(NotifyManager::Warning, data);
    consoleData->WarningHandler = ::make_scoped<NotifyWarningData>(handler);

    Data->Prepend(consoleData);
}

NotifyConsole::~NotifyConsole()
{
    delete ui;
}

void NotifyConsole::SetVisibility(ElementVisibilityFlags visibility)
{
    ui->BtnCloseConsole->setVisible(visibility.TestFlag(ElementVisibility_Close));
    ui->BtnClear->setVisible(visibility.TestFlag(ElementVisibility_Clear));
    ui->BtnShowWarnings->setVisible(visibility.TestFlag(ElementVisibility_ShowWarnings));
    ui->Filter->setVisible(visibility.TestFlag(ElementVisibility_Filter));
    ui->BtnShowInfo->setVisible(visibility.TestFlag(ElementVisibility_ShowInfos));
    ui->BtnShowErrors->setVisible(visibility.TestFlag(ElementVisibility_ShowErrors));
}

void NotifyConsole::SetCloseIcon(const IconsSvgIcon& icon)
{
    ui->BtnCloseConsole->setIcon(icon);
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

void NotifyConsole::AttachErrorsContainer(LocalPropertyErrorsContainer* container, const std::function<void (const Name&)>& handler)
{
    Q_ASSERT(!m_permanentErrors.contains(container));

    auto addError = [this, handler, container](const LocalPropertyErrorsContainerValue& error){
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
        error.Error->OnChanged.Connect(this, [this, pError, pConsoleData]{
            pConsoleData->Data->Body = pError->Native();
            m_updateErrors();
        }).MakeSafe(pConsoleData->ErrorHandler->Connections);
        if(pConsoleData->Data->Visible != nullptr){
            pConsoleData->Data->Visible->OnChanged.Connect(this, []{
                //update check role;
//                Edit
            }).MakeSafe(pConsoleData->ErrorHandler->Connections);
        }
        Data->Prepend(consoleData);
    };

    auto removeError = [this, handler, container](const LocalPropertyErrorsContainerValue& error) {
        m_permanentErrorsToErase[container].insert(error.Id);
        erasePermanentErrors();
    };

    for(const auto& error : *container) {
        addError(error);
    }

    container->OnErrorAdded.Connect(this, addError).MakeSafe(m_permanentErrors[container]);
    container->OnErrorRemoved.Connect(this, removeError).MakeSafe(m_permanentErrors[container]);
}

void NotifyConsole::erasePermanentErrors()
{
    m_erasePermanentErrors.Call([this]{
        if(m_permanentErrorsToErase.isEmpty()) {
            return;
        }

        qint32 index = 0;
        QSet<qint32> indicesToRemove;
        for(const auto& data : *Data) {
            if(data->ErrorHandler != nullptr) {
                auto foundIt = m_permanentErrorsToErase.find(data->ErrorHandler->Container);
                if(foundIt != m_permanentErrorsToErase.end()) {
                    if(foundIt.value().contains(data->ErrorHandler->Id)) {
                        indicesToRemove.insert(index);
                    }
                }
            }
            index++;
        }
        Data->Remove(indicesToRemove);
        m_permanentErrorsToErase.clear();
    });
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
    qint32 index = 0;
    QSet<qint32> indicesToRemove;
    for(const auto& data : *Data) {
        if(data->ErrorHandler == nullptr) {
            indicesToRemove.insert(index);
        }
        index++;
    }
    Data->Remove(indicesToRemove);
}

