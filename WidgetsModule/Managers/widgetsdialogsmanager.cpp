#include "widgetsdialogsmanager.h"

#include <QApplication>
#include <QFileDialog>
#include <QSettings>

WidgetsDialogsManager::WidgetsDialogsManager()
{

}

WidgetsDialogsManager& WidgetsDialogsManager::GetInstance()
{
    static WidgetsDialogsManager result;
    return result;
}

QList<QUrl> WidgetsDialogsManager::SelectDirectory(const QString& dialogHeader, bool isSaveMode, const QString& fileName, const QStringList& filters){
    QString searchDir(QString("last%1Folder").arg(isSaveMode ? "Save" : "Load"));
    QSettings internalSettings;
    auto lastSearchFolder = internalSettings.value(searchDir, QCoreApplication::applicationDirPath()).toString();
    QFileDialog fileDialog(qApp->activeWindow(), dialogHeader, lastSearchFolder);
    OnDialogCreated(&fileDialog);
    fileDialog.setNameFilters(filters);

    if(isSaveMode){ fileDialog.setAcceptMode(QFileDialog::AcceptSave); }
    fileDialog.selectFile(fileName);
    fileDialog.setFileMode(isSaveMode ? QFileDialog::AnyFile : QFileDialog::ExistingFiles);

    if(fileDialog.exec() == QDialog::Rejected) {
        return {};
    }

    internalSettings.setValue(searchDir, fileDialog.directoryUrl().path());
    return fileDialog.selectedUrls();
}
