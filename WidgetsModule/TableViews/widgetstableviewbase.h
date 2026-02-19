#ifndef WIDGETSTABLEVIEWBASE_H
#define WIDGETSTABLEVIEWBASE_H

#include <QTableView>

#include <SharedModule/internal.hpp>

class WidgetsTableViewBasePrivate;

class WidgetsTableViewBase : public QTableView
{
    Q_OBJECT
    using Super = QTableView;
    Q_PROPERTY(QColor selectionColor MEMBER m_selectionColor)
    Q_PROPERTY(QColor hoverColor MEMBER m_hoverColor)

public:
    WidgetsTableViewBase(QWidget* parent = nullptr);
    ~WidgetsTableViewBase();

    void setModel(QAbstractItemModel *model) override;

    void OverrideEditorEvent(const std::function<void (const FAction&)>& action);

private:
    Q_DECLARE_PRIVATE(WidgetsTableViewBase);
    void commitData(QWidget *editor) override;
    void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) override;
    void keyPressEvent(QKeyEvent *event) override;

    void paintEvent(QPaintEvent* event) override;
    void drawCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);
    void paintDropIndicator(QPainter *painter);
    void drawAndClipSpans(const QRegion &area, QPainter *painter,
                                         const QStyleOptionViewItem &option, QBitArray *drawn,
                                         int firstVisualRow, int lastVisualRow, int firstVisualColumn, int lastVisualColumn, const QColor& gridColor);
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QModelIndex m_editorIndex;
    LocalPropertyColor m_selectionColor;
    LocalPropertyColor m_hoverColor;
    QtLambdaConnections m_modelConnections;
};
#endif // WIDGETSTABLEVIEWBASE_H
