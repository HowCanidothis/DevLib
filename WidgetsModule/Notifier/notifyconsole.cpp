#include "notifyconsole.h"
#include "ui_notifyconsole.h"

#include <QStyledItemDelegate>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QSortFilterProxyModel>

#include "WidgetsModule/Utils/iconsmanager.h"

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
};

void RichTextItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem& inOption,
                                 const QModelIndex &index) const {
    QStyleOptionViewItem option = inOption;
    initStyleOption(&option, index);

    QStyle* style = option.widget? option.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(option.text);

    /// Painting item without text
    option.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    ctx.palette = option.palette;

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &option);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize RichTextItemDelegate::sizeHint(const QStyleOptionViewItem &inOption, const QModelIndex &index) const {
    QStyleOptionViewItem option = inOption;
    initStyleOption(&option, index);

    QTextDocument doc;
    doc.setHtml(option.text);
    doc.setTextWidth(option.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

class NotifyConsoleViewModel : public TModelsTableBase<NotifyConsoleDataWrapper>
{
    using Super = TModelsTableBase<NotifyConsoleDataWrapper>;
public:
    NotifyConsoleViewModel(QObject* parent)
        : Super(parent)
    {
        m_errorIcon = IconsManager::GetInstance().GetIcon("ErrorIcon");
        m_warningIcon = IconsManager::GetInstance().GetIcon("WarningIcon");
    }

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex&) const override
    {
        if(GetData() == nullptr) {
            return 0;
        }
        return GetData()->GetSize();
    }
    int columnCount(const QModelIndex&) const override
    {
        return 1;
    }
    QVariant data(const QModelIndex& index, int role) const override
    {
        if(!index.isValid()) {
            return QVariant();
        }

        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            return GetData()->At(index.row())->Data->GetData((NotifyData::Columns)(index.column() + 1));
        case Qt::DecorationRole:
            switch(GetData()->At(index.row())->Data->Type) {
            case NotifyManager::Warning: return m_warningIcon;
            case NotifyManager::Error: return m_errorIcon;
            default: break;
            }
        default:
            break;
        }

        return QVariant();
    }

private:
    IconsSvgIcon m_errorIcon;
    IconsSvgIcon m_warningIcon;
};

class ConsoleSortFilterViewModel : public QSortFilterProxyModel
{
    using Super = QSortFilterProxyModel;
public:
    ConsoleSortFilterViewModel(QObject* parent)
        : Super(parent)
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

private:
    NotifyConsoleViewModel* consoleModel() const { return reinterpret_cast<NotifyConsoleViewModel*>(sourceModel()); }

private:
    DelayedCallObject m_invalidate;
};

bool ConsoleSortFilterViewModel::filterAcceptsRow(int source_row, const QModelIndex&) const
{
    const auto& data = consoleModel()->GetData()->At(source_row);
    if(data->Data->Type < SeverityFilter) {
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
    , ui(new Ui::NotifyConsole)
{
    ui->setupUi(this);

    ui->BtnShowCategories->setVisible(false);
    auto viewModel = new NotifyConsoleViewModel(this);
    viewModel->SetData(Data);

    auto* filterModel = new ConsoleSortFilterViewModel(this);
    filterModel->setSourceModel(viewModel);
    ui->TableIssues->setModel(filterModel);

    ui->TableIssues->setItemDelegateForColumn(1, new RichTextItemDelegate(ui->TableIssues));

    IsOpened.Subscribe([this]{
        setVisible(IsOpened);
    });

    m_connectors.AddConnector<LocalPropertiesPushButtonConnector>(&IsShowWarnings, ui->BtnShowWarnings);
    m_connectors.AddConnector<LocalPropertiesLineEditConnector>(&filterModel->StringFilter, ui->Filter);
    IsShowWarnings.Subscribe([filterModel, this]{
        if(IsShowWarnings) {
            filterModel->SeverityFilter = NotifyManager::Warning;
        } else {
            filterModel->SeverityFilter = NotifyManager::Error;
        }
    });

    connect(ui->TableIssues, &QTableView::doubleClicked, [filterModel, this](const QModelIndex& current) {
        auto sourceIndex = filterModel->mapToSource(current);
        const auto& data = Data->At(sourceIndex.row());
        if(data->ErrorHandler != nullptr && data->ErrorHandler->Action != nullptr) {
            data->ErrorHandler->Action();
        }
    });
}

NotifyConsole::~NotifyConsole()
{
    delete ui;
}

void NotifyConsole::SetCloseIcon(const IconsSvgIcon& closeIcon)
{
    ui->BtnCloseConsole->setIcon(closeIcon);
}

void NotifyConsole::SetCleanIcon(const IconsSvgIcon& cleanIcon)
{
    ui->BtnClear->setIcon(cleanIcon);
}

void NotifyConsole::SetWarningIcon(const IconsSvgIcon& cleanIcon)
{
    ui->BtnShowWarnings->setIcon(cleanIcon);
}

void NotifyConsole::AttachErrorsContainer(LocalPropertyErrorsContainer* container, const std::function<void (const Name&)>& handler)
{
    Q_ASSERT(!m_permanentErrors.contains(container));

    auto addError = [this, handler, container](const LocalPropertyErrorsContainerValue& error){
        auto id = error.Id;
        auto consoleData = ::make_shared<NotifyConsoleData>();
        consoleData->ErrorHandler = new NotifyErrorContainerData( [handler, id]{ handler(id); }, container, id );
        consoleData->Data = ::make_shared<NotifyData>(NotifyManager::Error, error.Error);
        Data->Append(consoleData);
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

