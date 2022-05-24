#ifndef ADJUSTABLETABLEVIEW_H
#define ADJUSTABLETABLEVIEW_H

#include <QTableView>

#include <SharedModule/External/external.hpp>

class WidgetsAdjustableTableView : public QTableView
{
    using Super = QTableView;
public:
    WidgetsAdjustableTableView(QWidget* parent = nullptr);
    bool KeepStrictWidth;
    // QWidget interface
public:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void doItemsLayout() override;
    bool event(QEvent* event) override;
    void setModel(QAbstractItemModel* model) override;

private:
    void updateSizeHintCache() const;

private:
    mutable bool m_isDirty;
    mutable qint32 m_contentsWidth;
    mutable qint32 m_contentsHeight;
};

#endif // ADJUSTABLETABLEVIEW_H
