#ifndef DATABASEBROWSER_H
#define DATABASEBROWSER_H

#include <QMainWindow>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class DatabaseBrowser;
}

class DatabaseBrowser : public QMainWindow
{
    Q_OBJECT

public:
    explicit DatabaseBrowser(QWidget *parent = nullptr);
    ~DatabaseBrowser();

private slots:
    void on_BtnCreateTestDatabase_clicked();

    void on_BtnVacuum_clicked();

    void on_BtnClear_clicked();

    void on_BtnSave_clicked();

    void on_BtnLoad_clicked();

    void on_BtnLoadRecent_clicked();

    void on_BtnSaveCurrent_clicked();

    void on_BtnSettings_clicked();

private:
    Ui::DatabaseBrowser *ui;
    ScopedPointer<class DbDatabase> m_database;

    PropertiesDialogGeometryProperty m_settingsDialogGeometry;
    ScopedPointer<class QtQSSReader> m_qssReader;
    StringProperty m_qssFile;
    PointerProperty<DbDatabase> m_currentDatabase;
    LocalProperty<QString> m_currentDatabaseFileName;
    StringProperty m_lastSavedDatabaseFileName;
    StringProperty m_lastLoadDir;
    StringProperty m_lastSaveDir;

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
};

#endif // DATABASEBROWSER_H
