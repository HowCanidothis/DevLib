#include "databaseactionsscope.h"

#include <QFileDialog>
#include <QApplication>

#include <DatabaseEngine/internal.hpp>

DatabaseActionsScope::DatabaseActionsScope()
    : Super(Name("Database"))
    , m_currentDatabase("Database/Current/Database", nullptr)
    , m_lastSavedDatabaseFileName("SaveLoad/Last file", "")
    , m_lastLoadDir("SaveLoad/Load dir", "")
    , m_lastSaveDir("SaveLoad/Save dir", "")
{
    m_lastSavedDatabaseFileName.Subscribe([this]{
        m_loadRecentAction->setEnabled(!m_lastSavedDatabaseFileName.Native().isEmpty());
    });

    m_currentDatabase.Subscribe([this]{
        m_saveAction->setEnabled(m_currentDatabase != nullptr);
        m_vacuumAction->setEnabled(m_currentDatabase != nullptr);
        m_clearAction->setEnabled(m_currentDatabase != nullptr);
        m_database = m_currentDatabase;
    });

    m_currentDatabaseFileName.Subscribe([this]{
        m_saveCurrentAction->setEnabled(!m_lastSavedDatabaseFileName.Native().isEmpty());
    });
}

void DatabaseActionsScope::CreateActions()
{
     m_saveAction = createAction("Save", [this]{ saveAction(); });
     m_saveAction->setText(tr("Save"));
     m_saveAction->setIcon(QIcon(R"(D:\Projects\Qt\DevLib\Styles\metalcamstyle\icons\shaded.png)"));

     m_saveCurrentAction = createAction("SaveCurrent", [this]{ saveCurrentAction(); });
     m_saveCurrentAction->setText(tr("Save current"));
     m_saveCurrentAction->setEnabled(false);

     m_loadAction = createAction("Load", [this]{ loadAction(); });
     m_loadAction->setText(tr("Load"));

     m_loadRecentAction = createAction("LoadRecent", [this]{ loadRecentAction(); });
     m_loadRecentAction->setText(tr("Load recent"));

     m_vacuumAction = createAction("Vacuum", [this]{ vacuumAction(); });
     m_vacuumAction->setText(tr("Vacuum"));
     m_vacuumAction->setEnabled(false);

     m_clearAction = createAction("Clear", [this]{ clearAction(); });
     m_clearAction->setText(tr("Clear"));
     m_clearAction->setEnabled(false);

     m_showSettingsAction = createAction("ShowSettings", [this]{ showSettingsAction(); });
     m_showSettingsAction->setText("Settings");

     m_createTestDatabaseAction = createAction("CreateTestDatabase", [this]{ createTestDatabaseAction(); });
     m_createTestDatabaseAction->setText("Create test database");
}

void DatabaseActionsScope::saveAction()
{
    QFileDialog fileDialog(QApplication::activeWindow(), tr("Choose file to save database"), m_lastSaveDir, tr("Database files (*.cdb)"));
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

void DatabaseActionsScope::loadAction()
{
    QFileDialog fileDialog(QApplication::activeWindow(), tr("Choose file to load database"), m_lastLoadDir, tr("Database files (*.cdb)"));
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

void DatabaseActionsScope::createTestDatabaseAction()
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

void DatabaseActionsScope::vacuumAction()
{
    auto db = m_database.release();
    m_currentDatabase = nullptr;
    db->Vacuum();
    m_currentDatabase = db;
}

void DatabaseActionsScope::clearAction()
{
    auto db = m_database.release();
    m_currentDatabase = nullptr;
    db->RemoveAll();
    m_currentDatabase = db;
}

void DatabaseActionsScope::loadRecentAction()
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

void DatabaseActionsScope::saveCurrentAction()
{
    QFile saveFile(m_currentDatabaseFileName);
    if(saveFile.open(QFile::WriteOnly)) {
        QDataStream saveStream(&saveFile);
        m_currentDatabase->Save(saveStream);
    }
}

void DatabaseActionsScope::showSettingsAction()
{
    PropertiesDialog dialog("ProgramSettings", PropertiesSystem::Global, QApplication::activeWindow());
    dialog.Initialize();
    dialog.exec();
}
