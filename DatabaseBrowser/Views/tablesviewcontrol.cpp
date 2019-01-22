#include "tablesviewcontrol.h"
#include "ui_tablesviewcontrol.h"
#include <DatabaseEngine/internal.hpp>
#include <QInputDialog>
#include <QSortFilterProxyModel>

#include "Models/dbmodeltablestable.h"
#include "tableviewcontrol.h"

TablesViewControl::TablesViewControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TablesViewControl)
    , m_tableViewControl(new TableViewControl(this))
    , m_currentTable("Database/Current/Table", nullptr)
    , m_currentDatabase("Database/Current/Database", PropertiesSystem::Global)
    , m_currentHeader("Database/Current/Header", PropertiesSystem::Global)
{
    ui->setupUi(this);
    setEnabled(false);

    m_currentDatabase.Subscribe([this]{
        m_currentRows.Clear();
        ui->BtnAdd->setEnabled(false);
        ui->BtnRemove->setEnabled(false);
        ui->BtnEdit->setEnabled(false);

        auto model = ui->TablesView->model();
        if(model != nullptr) {
            ui->TablesView->setModel(nullptr);
            delete model;
        }

        if(m_currentDatabase != nullptr) {
            auto proxy = new QSortFilterProxyModel(this);
            m_model = new DbModelTablesTable(m_currentDatabase, proxy);
            connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SIGNAL(modelChanged()));
            proxy->setSourceModel(m_model);
            ui->TablesView->setModel(proxy);
            connect(ui->TablesView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged()));
        }
        setEnabled(m_currentDatabase != nullptr);
    });

    m_currentHeader.Subscribe([this]{
        ui->BtnAdd->setEnabled(m_currentHeader != nullptr);
    });

    m_currentRows.Subscribe([this]{
        if(m_currentRows.Size() == 1) {
            auto proxy = reinterpret_cast<QSortFilterProxyModel*>(ui->TablesView->model());
            m_currentTable = m_model->GetTable(proxy->mapToSource(proxy->index(*m_currentRows.begin(), 0)));
        } else {
            m_currentTable = nullptr;
        }
        ui->BtnEdit->setEnabled(m_currentRows.Size() == 1);
        ui->BtnRemove->setEnabled(!m_currentRows.IsEmpty());
    });

    connect(m_tableViewControl, SIGNAL(modelAboutToBeChanged()), this, SIGNAL(modelAboutToBeChanged()));
    connect(m_tableViewControl, SIGNAL(modelChanged()), this, SIGNAL(modelChanged()));
}

TablesViewControl::~TablesViewControl()
{
    delete ui;
}

void TablesViewControl::AboutToBeReset()
{
    emit m_model->layoutAboutToBeChanged();
}

void TablesViewControl::Reset()
{
    emit m_model->layoutChanged();
    ui->TablesView->reset();
    onSelectionChanged();
}

void TablesViewControl::on_BtnAdd_clicked()
{
    Q_ASSERT(m_currentHeader);
    bool ok;
    QString tableName = QInputDialog::getText(this, tr("Table creation dialog"), tr("Enter table name"), QLineEdit::Normal, "", &ok);
    if(ok) {
        if(tableName.isEmpty()) {
            qCCritical(LC_UI) << "Cannot create table with empty name";
            on_BtnAdd_clicked();
        }
        emit modelAboutToBeChanged();
        m_model->AddTable(m_currentHeader, tableName);
        emit modelChanged();
    }
}

void TablesViewControl::on_BtnRemove_clicked()
{
    QModelIndexList indexesToRemove;
    auto proxy = reinterpret_cast<QSortFilterProxyModel*>(ui->TablesView->model());
    for(const auto& row : m_currentRows) {
        indexesToRemove.append(proxy->mapToSource(proxy->index(row, 0)));
    }
    m_model->RemoveTables(indexesToRemove);
    onSelectionChanged();
}

void TablesViewControl::onSelectionChanged()
{
    auto selectedRows = ui->TablesView->selectionModel()->selectedIndexes();

    m_currentRows.SilentClear();

    for(const auto& mi : selectedRows) {
        if(!m_currentRows.IsContains(mi.row())) {
            m_currentRows.SilentInsert(mi.row());
        }
    }

    m_currentRows.Invoke();
}

void TablesViewControl::on_BtnEdit_clicked()
{
    m_tableViewControl->show();
}
