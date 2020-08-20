#include "widgetsadjustabletableview.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QTimer>
#include <QTimerEvent>

WidgetsAdjustableTableView::WidgetsAdjustableTableView(QWidget* parent)
    : Super(parent)
    , m_isDirty(true)
    , m_contentsWidth(0)
    , m_contentsHeight(0)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setVisible(false);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    addActions({ createAction("Dich", [this]{
                     auto selIndexes = selectedIndexes();
                     QSet<qint32> selectedRows;
                     for(const auto& selInd : selIndexes) {
                         selectedRows.insert(selInd.row());
                     }
                     if(!selectedRows.isEmpty()) {
                        model()->removeRow(*selectedRows.begin());
                     }
                 }) });
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

QSize WidgetsAdjustableTableView::minimumSizeHint() const
{
    updateSizeHintCache();
    return QSize(m_contentsWidth, m_contentsHeight);
}

QSize WidgetsAdjustableTableView::sizeHint() const
{
    updateSizeHintCache();
    return QSize(m_contentsWidth, m_contentsHeight);
}

void WidgetsAdjustableTableView::doItemsLayout()
{
    Super::doItemsLayout();
    m_isDirty = true;
    updateGeometry();
}

void WidgetsAdjustableTableView::updateSizeHintCache() const
{
    if(m_isDirty) {
        m_contentsWidth = 0;
        m_contentsHeight = 0;

        auto rowCount = model()->rowCount();
        if(rowCount != 0) {
            for(qint32 i(0); i < rowCount; i++) {
                m_contentsHeight += rowHeight(i);
            }
            auto columnCount = model()->columnCount();
            for(qint32 j(0); j < columnCount; j++) {
                m_contentsWidth += columnWidth(j);
            }

            m_contentsWidth += contentsMargins().left() + contentsMargins().right();
            m_contentsHeight += horizontalHeader()->sizeHint().height() + contentsMargins().top() + contentsMargins().bottom();
        }
        m_isDirty = false;
    }
}

bool WidgetsAdjustableTableView::event(QEvent* event)
{
    if(event->type() == QEvent::Timer) {
        m_isDirty = true;
        updateGeometry();
    }
    return Super::event(event);
}


void WidgetsAdjustableTableView::setModel(QAbstractItemModel* model)
{
    Super::setModel(model);
    updateGeometry();
}
