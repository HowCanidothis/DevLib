#ifndef TABLEVIEWCONTROL_H
#define TABLEVIEWCONTROL_H

#include <QWidget>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class TableViewControl;
}

class TableViewControl : public QWidget
{
    Q_OBJECT

public:
    explicit TableViewControl(QWidget *parent = nullptr);
    ~TableViewControl();

Q_SIGNALS:
    void modelAboutToBeChanged();
    void modelChanged();

public Q_SLOTS:
    void AboutToBeReset();
    void Reset();

private slots:
    void on_BtnAddRows_clicked();

    void on_BtnRemoveRows_clicked();

    void onSelectionChanged();

private:
    Ui::TableViewControl *ui;

    LocalPropertySet<qint32> m_currentRows;
    PointerPropertyPtr<class DbTable> m_currentTable;
    class DbModelTableTable* m_model;
};

#endif // TABLEVIEWCONTROL_H
