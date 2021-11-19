#include "widgetsadjustabletableview.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QTimer>
#include <QTimerEvent>

#include "WidgetsModule/Utils/widgethelpers.h"

WidgetsAdjustableTableView::WidgetsAdjustableTableView(QWidget* parent)
    : Super(parent)
    , m_isDirty(true)
    , m_contentsWidth(0)
    , m_contentsHeight(0)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    WidgetsAttachment::Attach(this, [this](QObject*, QEvent* event){
        if(event->type() == QEvent::Show || event->type() == QEvent::StyleChange) {
            m_isDirty = true;
            updateGeometry();
        }
        return false;
    });
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

        if(model() != nullptr) {
            auto rowCount = model()->rowCount();
            if(rowCount != 0) {
                for(qint32 i(0); i < rowCount; i++) {
                    m_contentsHeight += rowHeight(i);
                }
                auto columnCount = model()->columnCount();
                for(qint32 j(0); j < columnCount; j++) {
                    m_contentsWidth += columnWidth(j);
                }

                if(verticalHeader()->isVisible()) {
                    m_contentsWidth += verticalHeader()->sizeHint().width();
                }
                m_contentsWidth += contentsMargins().left() + contentsMargins().right();
                if(horizontalHeader()->isVisible()) {
                    m_contentsHeight += horizontalHeader()->sizeHint().height();
                }
                m_contentsHeight += contentsMargins().top() + contentsMargins().bottom();
            }
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
