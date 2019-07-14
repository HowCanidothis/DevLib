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

private:
    Ui::DatabaseBrowser *ui;

    PropertiesDialogGeometryProperty m_settingsDialogGeometry;
    ScopedPointer<class QtQSSReader> m_qssReader;
    StringProperty m_qssFile;
    ScopedPointer<class DatabaseActionsScope> m_databaseActionsScope;

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
};

#endif // DATABASEBROWSER_H
