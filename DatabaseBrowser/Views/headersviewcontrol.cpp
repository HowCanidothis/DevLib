#include "headersviewcontrol.h"
#include "ui_headersviewcontrol.h"

#include <QInputDialog>

#include <DatabaseEngine/internal.hpp>

#include "Models/dbmodelheadstree.h"

HeadersViewControl::HeadersViewControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HeadersViewControl)
    , m_headersDialogGeometry("Headers")
    , m_FieldsDialogGeometry("Fields")
    , m_currentDatabase("Database/Current/Database", PropertiesSystem::Global)
    , m_currentHeader("Database/Current/Header", nullptr)
    , m_currentField(nullptr)
{
    ui->setupUi(this);
    setEnabled(false);
    ui->BtnEdit->setEnabled(false);
    ui->BtnRemove->setEnabled(false);

    ui->HeadersView->selectionModel();
    m_currentDatabase.Subscribe([this]{
        m_currentHeader = nullptr;
        m_currentField = nullptr;
        auto model = ui->HeadersView->model();
        if(model != nullptr) {
            ui->HeadersView->setModel(nullptr);
            delete model;
        }

        if(m_currentDatabase != nullptr) {
            model = new DbModelHeadsTree(m_currentDatabase, this);
            ui->HeadersView->setModel(model);
            connect(ui->HeadersView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged()));
            ui->HeadersView->expandAll();
        }
        setEnabled(m_currentDatabase != nullptr);
    });

    m_selectedRows.Subscribe([this]{
        validateControlsHeaders();
    });
}

HeadersViewControl::~HeadersViewControl()
{
    delete ui;
}

void HeadersViewControl::on_BtnAdd_clicked()
{
    Q_ASSERT(m_currentDatabase);


    auto header = m_currentHeader != nullptr ? m_currentHeader :
                  m_currentField != nullptr ? model()->AsHead(m_selectedRows.begin()->parent()) : nullptr;

    if(header != nullptr) {
        QString name, defaultValue;
        qint32 type;
        if(execFieldDialog(tr("Creation field dialog"), name, type, defaultValue) == QDialog::Accepted) {
            emit modelAboutToBeChanged();
            model()->AddFieldToHeader(header, type, name, defaultValue);
            emit modelChanged();
        }
    } else {
        QString name;
        if(execHeaderDialog(tr("Creation header dialog"), name) == QDialog::Accepted) {
            emit modelAboutToBeChanged();
            model()->AddTableHeader(name);
            emit modelChanged();
        }
    }
}

void HeadersViewControl::on_BtnRemove_clicked()
{
    Q_ASSERT(!m_selectedRows.IsEmpty());

    emit modelAboutToBeChanged();
    if(m_selectedRows.Size() == 1) {
        if(m_currentHeader != nullptr && m_currentField == nullptr) {
            model()->RemoveTableHead(ui->HeadersView->currentIndex());
        } else if(m_currentField != nullptr) {
            model()->RemoveTableField(ui->HeadersView->currentIndex());
        }
    } else {
        QModelIndexList indexesToRemove;
        for(const auto& selectedRow : m_selectedRows) {
            indexesToRemove.append(selectedRow);
        }
        model()->Remove(indexesToRemove);
    }
    emit modelChanged();
    onSelectionChanged();
}

void HeadersViewControl::onSelectionChanged()
{
    auto selectedRows = ui->HeadersView->selectionModel()->selectedRows(0);
    m_selectedRows.SilentClear();
    for(const auto& rowIndex : selectedRows) {
        m_selectedRows.SilentInsert(rowIndex);
    }

    if(selectedRows.size() != 1) {
        m_currentHeader = nullptr;
        m_currentField = nullptr;
    } else {
        auto headersModel = reinterpret_cast<DbModelHeadsTree*>(ui->HeadersView->model());

        m_currentField = headersModel->AsField(selectedRows.first(), m_currentFieldHeader);
        if(m_currentField != nullptr) {
            m_currentHeader = headersModel->AsHead(selectedRows.first().parent());
        } else {
            m_currentHeader = headersModel->AsHead(selectedRows.first());
        }
    }

    m_selectedRows.Invoke();
}

DbModelHeadsTree* HeadersViewControl::model() const
{
    return reinterpret_cast<DbModelHeadsTree*>(ui->HeadersView->model());
}

bool HeadersViewControl::hasCurrentFieldOrCurrentHeader() const
{
    return m_currentField != nullptr || m_currentHeader != nullptr;
}

#define SWITCH_TYPE_NAME(Cpp, Db) databaseSortedTypes.insert(DbTableFieldTypeHelper<Cpp>::Type, DbTableFieldTypeHelper<Cpp>::GetDbTypeString());
qint32 HeadersViewControl::execFieldDialog(const QString& caption, QString& name, qint32& type, QString& defaultValue)
{
    auto propertiesInitHandler = [&] {
        new ExternalStringProperty("Name", name);

        QMap<DbTableFieldType, QString> databaseSortedTypes;

        DB_FOREACH_COMPLEX_FIELDS(SWITCH_TYPE_NAME)
        DB_FOREACH_POD_FIELDS(SWITCH_TYPE_NAME)
        DB_FOREACH_PRIMITIVE_FIELDS(SWITCH_TYPE_NAME)

        (new ExternalNamedUIntProperty("Type", type))->SetNames(databaseSortedTypes.values());

        new ExternalStringProperty("Default value", defaultValue);
    };

    PropertiesDialog dialog("Fields", PropertiesSystem::Temp, this);
    dialog.setWindowTitle(caption);
    dialog.ChangeOptions().AddFlag(PropertiesDialogBase::Option_ClearContextOnDone);
    dialog.Initialize([this, propertiesInitHandler]{
        propertiesInitHandler();
    });
    return dialog.exec();
}

qint32 HeadersViewControl::execHeaderDialog(const QString& caption, QString& name)
{
    auto propertiesInitHandler = [&name] {
        new ExternalStringProperty("Name", name);
    };

    PropertiesDialog dialog("Headers", PropertiesSystem::Temp, this);
    dialog.setWindowTitle(caption);
    dialog.ChangeOptions().AddFlag(PropertiesDialogBase::Option_ClearContextOnDone);
    dialog.Initialize([this, propertiesInitHandler]{
        propertiesInitHandler();
    });
    return dialog.exec();
}

void HeadersViewControl::validateControlsHeaders()
{
    bool isSetControlsEnabled = false;

    ui->BtnAdd->setText("");
    ui->BtnEdit->setText("");
    ui->BtnRemove->setText("");

    ui->BtnAdd->setEnabled(false);
    ui->BtnEdit->setEnabled(false);
    ui->BtnRemove->setEnabled(false);

    if(m_selectedRows.Size() > 1) {
        ui->BtnRemove->setText(tr("Remove selected"));
        ui->BtnAdd->setEnabled(false);
        ui->BtnEdit->setEnabled(false);
        ui->BtnRemove->setEnabled(true);
    } else if(m_currentHeader != nullptr && m_currentField == nullptr) {
        ui->BtnAdd->setText(tr("Add new field"));
        ui->BtnEdit->setText(tr("Edit header"));
        ui->BtnRemove->setText(tr("Remove header"));
        isSetControlsEnabled = true;
    } else if(m_currentField != nullptr) {
        ui->BtnAdd->setText(tr("Add new field"));
        ui->BtnEdit->setText(tr("Edit field"));
        ui->BtnRemove->setText(tr("Remove field"));
        isSetControlsEnabled = true;
    } else { // nothing is selected
        ui->BtnAdd->setText(tr("Add new header"));
        ui->BtnAdd->setEnabled(true);
    }

    if(isSetControlsEnabled) {
        ui->BtnAdd->setEnabled(true);
        ui->BtnEdit->setEnabled(true);
        ui->BtnRemove->setEnabled(true);
    }
}

void HeadersViewControl::on_BtnEdit_clicked()
{
    Q_ASSERT(hasCurrentFieldOrCurrentHeader());

    if(m_currentHeader != nullptr && m_currentField == nullptr) {
        QString name = m_currentHeader.Native()->GetName().ToString();

        if(execHeaderDialog(tr("Editing header dialog"), name) == QDialog::Accepted) {
            emit modelAboutToBeChanged();
            model()->EditTableHeader(m_currentHeader, name);
            emit modelChanged();
        }
    } else { // if(m_currentField != nullptr)

        QString name = m_currentField->Name.ToString();
        QString defaultValue = m_currentField->DefaultValue.ToString();
        qint32 type = m_currentField->Type;

        if(execFieldDialog(tr("Editing field dialog"), name, type, defaultValue) == QDialog::Accepted) {
            emit modelAboutToBeChanged();
            model()->EditTableField(m_currentHeader, m_currentField, type, name, defaultValue);
            emit modelChanged();
        }
    }
}
