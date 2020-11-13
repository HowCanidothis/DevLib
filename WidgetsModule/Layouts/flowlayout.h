#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QStyle>

// Based on Qt sample
class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(int margin = 0, int hSpacing = 0, int vSpacing = 0);
    ~FlowLayout();

    void RemoveWidgets(const QSet<QWidget*>& widgets);
    void addItem(QLayoutItem* item) override;
    void SetHorizontalSpacing(qint32 spacing) { setSpacing(spacing); }
    void SetVerticalSpacing(qint32 spacing) { m_vSpace = spacing; }
    int HorizontalSpacing() const { return spacing(); }
    int VerticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QLayoutItem* takeAt(int index) override;

private:
    int doLayout(const QRect& rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    ArrayPointers<QLayoutItem> m_itemList;
    int m_vSpace;
};

#endif // FLOWLAYOUT_H
