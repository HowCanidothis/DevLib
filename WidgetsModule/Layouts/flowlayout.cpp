#include <QtWidgets>

#include "flowlayout.h"

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_vSpace(vSpacing)
{
    setSpacing(hSpacing);
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
}

void FlowLayout::RemoveWidgets(const QSet<QWidget*>& widgets)
{
    auto end = std::remove_if(m_itemList.begin(), m_itemList.end(), [&widgets](QLayoutItem* item) {
        return widgets.contains(item->widget());
    });
    m_itemList.Resize(std::distance(m_itemList.begin(), end));
}

void FlowLayout::addItem(QLayoutItem* item)
{
    m_itemList.Append(item);
}

int FlowLayout::VerticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int FlowLayout::count() const
{
    return m_itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    if (index >= 0 && index < m_itemList.size()) {
        return m_itemList.At(index);
    }
    return nullptr;
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < m_itemList.size()) {
        auto* result = m_itemList.At(index);
        m_itemList.RemoveByIndex(index);
        return result;
    }
    else
        return 0;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return 0;
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void FlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for(auto* item : m_itemList) {
        size = size.expandedTo(item->minimumSize());
    }
    size += QSize(2*margin(), 2*margin());
    return size;
}

int FlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;


    struct RowInfo
    {
        qint32 ItemsWidth = 0;
        qint32 Y;
        QVector<QLayoutItem*> Items;
    };

    QMap<qint32, RowInfo> rows;
    qint32 currentRow = 0;
    int spaceX = spacing();
    int spaceY = VerticalSpacing();

    for(auto* item : m_itemList) {
        auto nextX = x + item->sizeHint().width() + spaceX;
        if(nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            auto& prevRow = rows[currentRow];
            prevRow.ItemsWidth += (prevRow.Items.size() - 1) * spaceX;
            currentRow++;
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
        }
        auto& row = rows[currentRow];
        row.Items.append(item);
        row.Y = y;
        row.ItemsWidth += item->sizeHint().width();

        x = nextX;

        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    if(!testOnly) {
        if(!rows.isEmpty()) {
            auto& row = rows.last();
            row.ItemsWidth += (row.Items.size() - 1) * spaceX;
        }

        for(const auto& row : rows) {
            x = effectiveRect.x() /*+ (effectiveRect.width() - row.ItemsWidth) / 2*/;
            for(auto* item : row.Items) {
                item->setGeometry(QRect(QPoint(x, row.Y), item->sizeHint()));
                x += item->sizeHint().width() + spaceX;
            }
        }
    }

    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, 0, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}
