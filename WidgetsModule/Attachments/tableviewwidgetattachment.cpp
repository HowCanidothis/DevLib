#include "tableviewwidgetattachment.h"

#include <QHeaderView>

TableViewColumnsWidgetAttachment::TableViewColumnsWidgetAttachment(QTableView* targetTableView)
    : Super(targetTableView)
    , m_targetTableView(targetTableView)
{
    m_attachmentWidgets.Resize(targetTableView->model()->columnCount());

    for(auto& widget : m_attachmentWidgets) {
        widget = nullptr;
    }
    targetTableView->viewport()->installEventFilter(this);
}

void TableViewColumnsWidgetAttachment::SetWidget(qint32 columnIndex, QWidget* widget)
{
    Q_ASSERT(m_attachmentWidgets.Size() > columnIndex);
    auto*& attachedWidget = m_attachmentWidgets[columnIndex];
    delete attachedWidget;
    attachedWidget = widget;
    widget->setParent(m_targetTableView->horizontalHeader());
}

bool TableViewColumnsWidgetAttachment::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::Paint) {
        auto* viewport = qobject_cast<QWidget*>(watched);
        auto* tableView = qobject_cast<QTableView*>(viewport->parentWidget());
        if(tableView != nullptr) {
            qint32 columnsCount = tableView->model()->columnCount();
            for(qint32 i(0); i < columnsCount; ++i) {
                auto* attachmentWidget = m_attachmentWidgets.At(i);
                if(attachmentWidget != nullptr) {
                    QHeaderView* headerView = tableView->horizontalHeader();
                    auto x = headerView->sectionViewportPosition(i);
                    auto y = 0;
                    attachmentWidget->setGeometry(x + 2,y, headerView->sectionSize(i) - 4, headerView->height());
                    attachmentWidget->setCursor(QCursor(Qt::ArrowCursor));
                }
            }
        }
    }

    return false;
}

qint32 TableViewColumnsWidgetAttachment::IndexOf(QWidget* widget) const
{
    auto foundIt = std::find(m_attachmentWidgets.begin(), m_attachmentWidgets.end(), widget);
    if(foundIt != m_attachmentWidgets.end()) {
        return std::distance(m_attachmentWidgets.begin(), foundIt);
    }
    return -1;
}


TableViewColumnsWidgetAttachmentDynamic::TableViewColumnsWidgetAttachmentDynamic(QTableView* targetTableView)
    : m_targetTableView(targetTableView)
    , m_createDelegate(nullptr)
{
}

void TableViewColumnsWidgetAttachmentDynamic::Initialize(const TableViewColumnsWidgetAttachmentDynamic::CreateDelegate& createDelegate)
{
    Q_ASSERT(m_createDelegate == nullptr);
    m_createDelegate = createDelegate;
    adjustAttachments(0, m_targetTableView->horizontalHeader()->count());
    connect(m_targetTableView->horizontalHeader(), &QHeaderView::sectionCountChanged, this, &TableViewColumnsWidgetAttachmentDynamic::adjustAttachments);
    m_targetTableView->viewport()->installEventFilter(this);
}

void TableViewColumnsWidgetAttachmentDynamic::SetVisible(bool visible)
{
    for(auto* attachment : m_attachmentWidgets) {
        attachment->setVisible(visible);
    }
}

bool TableViewColumnsWidgetAttachmentDynamic::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::Paint) {
        auto* viewport = qobject_cast<QWidget*>(watched);
        auto* tableView = qobject_cast<QTableView*>(viewport->parentWidget());
        if(tableView != nullptr) {
            qint32 columnsCount = tableView->model()->columnCount();
            for(qint32 i(0); i < columnsCount; ++i) {
                auto foundIt = m_attachmentWidgets.find(i);
                if(foundIt != m_attachmentWidgets.end()) {
                    QHeaderView* headerView = tableView->horizontalHeader();
                    auto x = headerView->sectionViewportPosition(i);
                    auto y = 0;
                    foundIt.value()->setGeometry(x + 2,y, headerView->sectionSize(i) - 4, headerView->height());
                    foundIt.value()->setCursor(QCursor(Qt::ArrowCursor));
                }
            }
        }
    }

    return false;
}

void TableViewColumnsWidgetAttachmentDynamic::adjustAttachments(qint32 oldCount, qint32 newCount)
{
    if(oldCount < newCount) {
        qint32 counter = oldCount;
        while(counter != newCount) {
            auto* attachment = m_createDelegate();
            attachment->setParent(m_targetTableView->horizontalHeader());
            m_attachmentWidgets.insert(counter, attachment);
            counter++;
        }
    } else if(oldCount > newCount) {
        qint32 counter = newCount;
        while(counter != oldCount) {
            auto foundIt = m_attachmentWidgets.find(counter);
            if(foundIt != m_attachmentWidgets.end()) {
                delete foundIt.value();
                m_attachmentWidgets.erase(foundIt);
            }
            counter++;
        }
    }
}
