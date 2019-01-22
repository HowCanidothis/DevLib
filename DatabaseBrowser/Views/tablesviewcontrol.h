#ifndef TABLESVIEWCONTROL_H
#define TABLESVIEWCONTROL_H

#include <QWidget>
#include <PropertiesModule/internal.hpp>

namespace Ui {
class TablesViewControl;
}

class TablesViewControl : public QWidget
{
    Q_OBJECT

public:
    explicit TablesViewControl(QWidget *parent = nullptr);
    ~TablesViewControl();

Q_SIGNALS:
    void modelAboutToBeChanged();
    void modelChanged();

public Q_SLOTS:
    void AboutToBeReset();
    void Reset();

private slots:
    void on_BtnAdd_clicked();

    void on_BtnRemove_clicked();

    void onSelectionChanged();

    void on_BtnEdit_clicked();

private:
    Ui::TablesViewControl *ui;

    class TableViewControl* m_tableViewControl;
    PointerProperty<class DbTable> m_currentTable;
    PointerPropertyPtr<class DbDatabase> m_currentDatabase;
    PointerPropertyPtr<class DbTableHeader> m_currentHeader;
    LocalPropertySet<qint32> m_currentRows;

    class DbModelTablesTable* m_model;
};

#endif // TABLEVIEWCONTROL_H
