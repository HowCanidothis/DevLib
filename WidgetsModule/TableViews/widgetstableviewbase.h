#ifndef WIDGETSTABLEVIEWBASE_H
#define WIDGETSTABLEVIEWBASE_H

#include <QTableView>

#include <SharedModule/internal.hpp>

class WidgetsTableViewBase : public QTableView
{
    Q_OBJECT
    using Super = QTableView;
public:
    WidgetsTableViewBase(QWidget* parent = nullptr);

    void OverrideEditorEvent(const std::function<void (const FAction&)>& action);

private:
    void commitData(QWidget *editor) override;
    void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QModelIndex m_editorIndex;
};
#endif // WIDGETSTABLEVIEWBASE_H
