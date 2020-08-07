#ifndef WIDGETSACTIVETABLEVIEWBASE_H
#define WIDGETSACTIVETABLEVIEWBASE_H

#include <QTableView>

#include <SharedModule/internal.hpp>

class WidgetsActiveTableViewBase : public QTableView
{
    using Super = QTableView;
public:
    WidgetsActiveTableViewBase(QWidget* parent = nullptr);

    template<class T>
    T* GetModel() const { return reinterpret_cast<T*>(model()); }

    Dispatcher OnActivated;
    CommonDispatcher<qint32> OnSelectionChanged;

    // QWidget interface
protected:
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
    void mousePressEvent(QMouseEvent* event) override;
};
#endif // WIDGETSACTIVETABLEVIEWBASE_H
