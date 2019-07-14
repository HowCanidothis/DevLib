#ifndef DATABASEACTIONS_H
#define DATABASEACTIONS_H

#include <PropertiesModule/internal.hpp>
#include <ActionsModule/internal.hpp>

class DatabaseActionsScope : public ActionsScopeBase
{
    typedef ActionsScopeBase Super;
public:
    DatabaseActionsScope();

    void CreateActions();

private:
    ScopedPointer<class DbDatabase> m_database;

    PointerProperty<class DbDatabase> m_currentDatabase;
    LocalProperty<QString> m_currentDatabaseFileName;
    StringProperty m_lastSavedDatabaseFileName;
    StringProperty m_lastLoadDir;
    StringProperty m_lastSaveDir;

private:
    Action* m_saveCurrentAction;
    Action* m_saveAction;
    Action* m_loadAction;
    Action* m_loadRecentAction;
    Action* m_vacuumAction;
    Action* m_clearAction;
    Action* m_showSettingsAction;
    Action* m_createTestDatabaseAction;

private:
    void saveAction();
    void loadAction();
    void createTestDatabaseAction();
    void vacuumAction();
    void clearAction();
    void loadRecentAction();
    void saveCurrentAction();
    void showSettingsAction();
};

#endif // DATABASEACTIONS_H
