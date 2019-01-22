#include "databasebrowser.h"
#include "ui_databasebrowser.h"

#include <QFileDialog>

#include <SharedModule/external/qtqssreader.h>
#include <DatabaseEngine/internal.hpp>

#include "Models/modelmemorytree.h"
#include "Models/dbmodeltabletable.h"

DatabaseBrowser::DatabaseBrowser(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DatabaseBrowser)
    , m_settingsDialogGeometry("ProgramSettings")
    , m_qssFile("Style/Path", "")
    , m_currentDatabase("Database/Current/Database", nullptr)
    , m_lastSavedDatabaseFileName("SaveLoad/Last file", "")
    , m_lastLoadDir("SaveLoad/Load dir", "")
    , m_lastSaveDir("SaveLoad/Save dir", "")
{
    ui->setupUi(this);

    connect(ui->HeadersVC, SIGNAL(modelAboutToBeChanged()), ui->TablesVC, SLOT(AboutToBeReset()));
    connect(ui->HeadersVC, SIGNAL(modelChanged()), ui->TablesVC, SLOT(Reset()));
    connect(ui->HeadersVC, SIGNAL(modelAboutToBeChanged()), ui->MemoryVC, SLOT(AboutToBeReset()));
    connect(ui->HeadersVC, SIGNAL(modelChanged()), ui->MemoryVC, SLOT(Reset()));
    connect(ui->TablesVC, SIGNAL(modelAboutToBeChanged()), ui->MemoryVC, SLOT(AboutToBeReset()));
    connect(ui->TablesVC, SIGNAL(modelChanged()), ui->MemoryVC, SLOT(Reset()));

    m_lastSavedDatabaseFileName.Subscribe([this]{
        ui->BtnLoadRecent->setEnabled(!m_lastSavedDatabaseFileName.Native().isEmpty());
    });

    m_currentDatabase.Subscribe([this]{
        ui->BtnSave->setEnabled(m_currentDatabase != nullptr);
        ui->BtnVacuum->setEnabled(m_currentDatabase != nullptr);
        ui->BtnClear->setEnabled(m_currentDatabase != nullptr);
        m_database = m_currentDatabase;
    });

    m_currentDatabaseFileName.Subscribe([this]{
        ui->BtnSaveCurrent->setEnabled(!m_lastSavedDatabaseFileName.Native().isEmpty());
    });


    PropertiesSystem::Load("app.ini", PropertiesSystem::Global);

    m_qssReader = new QtQSSReader;
    m_qssReader->SetEnableObserver(true);
    m_qssFile.Subscribe([this]{
        m_qssReader->Install(m_qssFile);
    });
}

DatabaseBrowser::~DatabaseBrowser()
{
    PropertiesSystem::Save("app.ini", PropertiesSystem::Global);
    delete ui;
}

void DatabaseBrowser::on_BtnCreateTestDatabase_clicked()
{
    DbDatabaseProperties dbProperties;
    auto db = DbDatabase::CreateDatabase(dbProperties);

    auto header = db->AddHeader("Default header");
    header->AddField<QString>("name", "My name is Andrey");
    header->AddField<bool>("boolean", "false");
    header->AddField<QString>("nameAfter", "I'm a developer");
    header->AddField<Array<Point3D>>("points", "(1,2,3);(2,3,1)");

    auto table = db->AddTable(header, "Values");

    auto row = table->Append();
    table->Change<DbString>(row, 1) = "BBBB";
    table->Change<bool>(row, 2) = false;
    table->Change<DbString>(row, 3) = "BBBB";
    row = table->Append();
    table->Change<DbString>(row, 1) = "AAAA";
    table->Change<bool>(row, 2) = true;
    table->Change<DbString>(row, 3) = "BBBB";

    Array<Point3D> points;
    table->At<DbPoint3DArray>(row, 4).ToNative(points);
    points.Append({0.0, 213.0, 32.0});
    table->Change<DbPoint3DArray>(row, 4) = points;

    row = table->Append();

    DbModificationRule rule(header);
    rule.EditField<qint32>(header->GetField("boolean"), "boolean", "123");

    db->Edit(rule);

    row = table->Append();

    table->Sort(1);
    table->SaveProxies();

    m_currentDatabase = db;
}

void DatabaseBrowser::on_BtnVacuum_clicked()
{
    auto db = m_database.release();
    m_currentDatabase = nullptr;
    db->Vacuum();
    m_currentDatabase = db;
}

void DatabaseBrowser::on_BtnClear_clicked()
{
    auto db = m_database.release();
    m_currentDatabase = nullptr;
    db->RemoveAll();
    m_currentDatabase = db;
}

void DatabaseBrowser::on_BtnSave_clicked()
{
    QFileDialog fileDialog(this, tr("Choose file to save database"), m_lastSaveDir, tr("Database files (*.cdb)"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setDefaultSuffix("cdb");

    if(fileDialog.exec()) {
        auto selectedFiles = fileDialog.selectedFiles();
        Q_ASSERT(selectedFiles.size() == 1);

        QFile saveFile(selectedFiles.first());
        if(saveFile.open(QFile::WriteOnly)) {
            QDataStream saveStream(&saveFile);
            m_currentDatabase->Save(saveStream);
            m_lastSaveDir = fileDialog.directory().absolutePath();
            m_lastSavedDatabaseFileName = saveFile.fileName();
            m_currentDatabaseFileName = m_lastSavedDatabaseFileName;
        }
    }
}

void DatabaseBrowser::on_BtnLoad_clicked()
{
    QFileDialog fileDialog(this, tr("Choose file to load database"), m_lastLoadDir, tr("Database files (*.cdb)"));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::AnyFile);

    if(fileDialog.exec()) {
        auto selectedFiles = fileDialog.selectedFiles();
        Q_ASSERT(selectedFiles.size() == 1);

        m_currentDatabase = nullptr;

        QFile loadFile(selectedFiles.first());
        if(loadFile.open(QFile::ReadOnly)) {
            DbDatabaseProperties dbProperties;
            auto db = DbDatabase::CreateDatabase(dbProperties);

            QDataStream loadStream(&loadFile);
            if(db->Load(loadStream)) {
                m_lastLoadDir = fileDialog.directory().absolutePath();
                m_lastSavedDatabaseFileName = loadFile.fileName();
                m_currentDatabaseFileName = m_lastSavedDatabaseFileName;
                m_currentDatabase = db;
            } else {
                delete db;
            }
        }
    }
}

void DatabaseBrowser::on_BtnLoadRecent_clicked()
{
    QFile file(m_lastSavedDatabaseFileName);

    if(file.open(QFile::ReadOnly)) {
        QDataStream loadStream(&file);
        DbDatabaseProperties dbProperties;

        m_currentDatabase = nullptr;

        auto db = DbDatabase::CreateDatabase(dbProperties);

        if(db->Load(loadStream)) {
            m_currentDatabase = db;
            m_currentDatabaseFileName = file.fileName();
        } else {
            delete db;
        }
    }
}

void DatabaseBrowser::on_BtnSaveCurrent_clicked()
{
    QFile saveFile(m_currentDatabaseFileName);
    if(saveFile.open(QFile::WriteOnly)) {
        QDataStream saveStream(&saveFile);
        m_currentDatabase->Save(saveStream);
    }
}

void DatabaseBrowser::on_BtnSettings_clicked()
{
    PropertiesDialog dialog("ProgramSettings", PropertiesSystem::Global, this);
    dialog.Initialize();
    dialog.exec();
}


void DatabaseBrowser::showEvent(QShowEvent* )
{
    m_qssReader->Install(m_qssFile);
}
