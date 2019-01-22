#include "tableviewcontrol.h"
#include "ui_tableviewcontrol.h"

#include <QSortFilterProxyModel>
#include <QInputDialog>

#include "Models/dbmodeltabletable.h"

TableViewControl::TableViewControl(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::TableViewControl)
    , m_currentTable("Database/Current/Table", [this]{
        auto model = ui->Table->model();
        if(model != nullptr) {
            ui->Table->setModel(nullptr);
            delete model;
        }

        if(m_currentTable != nullptr) {
            m_model = new DbModelTableTable(m_currentTable);
            connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SIGNAL(modelChanged()));
            connect(m_model, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)), this, SIGNAL(modelAboutToBeChanged()));
            connect(m_model, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)), this, SIGNAL(modelChanged()));
            ui->Table->setModel(m_model);
            connect(ui->Table->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged()));
            ui->BtnAddRows->setEnabled(true);
        } else {
            m_model = nullptr;
            ui->BtnAddRows->setEnabled(false);
        }
    }, PropertiesSystem::Global)
{
    ui->setupUi(this);

    m_currentRows.Subscribe([this]{
        ui->BtnRemoveRows->setEnabled(!m_currentRows.IsEmpty());
    });
}

void TableViewControl::AboutToBeReset()
{
    emit m_model->layoutAboutToBeChanged();
}

void TableViewControl::Reset()
{
    emit m_model->layoutChanged();
    ui->Table->reset();
}

TableViewControl::~TableViewControl()
{
    delete ui;
}

void TableViewControl::on_BtnAddRows_clicked()
{
    bool ok;
    qint32 rowCount = QInputDialog::getInt(this, tr("Adding new rows to table"), tr("How many rows you want to insert"), 1, 0, 100000, 1, &ok);

    if(ok) {
        AboutToBeReset();
        m_model->Add(rowCount);
        Reset();
    }
}

void TableViewControl::on_BtnRemoveRows_clicked()
{
    QModelIndexList toRemove;
    for(auto row : m_currentRows) {
        toRemove.append(m_model->index(row, 0));
    }

    AboutToBeReset();
    m_model->Remove(toRemove);
    Reset();
    onSelectionChanged();
}

void TableViewControl::onSelectionChanged()
{
    auto selectedRows = ui->Table->selectionModel()->selectedIndexes();

    m_currentRows.SilentClear();

    for(const auto& mi : selectedRows) {
        if(!m_currentRows.IsContains(mi.row())) {
            m_currentRows.SilentInsert(mi.row());
        }
    }

    m_currentRows.Invoke();
}
