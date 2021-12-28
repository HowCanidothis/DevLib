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

QList<QUrl> WidgetsDialogsManager::SelectDirectory(const DescImportExportSourceParams& params){
    QString searchDir(QString("last%1Folder").arg(params.Mode == DescImportExportSourceParams::Save ? "Save" : "Load"));
    QSettings internalSettings;
    auto lastSearchFolder = internalSettings.value(searchDir, QCoreApplication::applicationDirPath()).toString();
    QFileDialog fileDialog(qApp->activeWindow(), params.Label, lastSearchFolder);
    OnDialogCreated(&fileDialog);
    fileDialog.setNameFilters(params.Filters);
    fileDialog.setDefaultSuffix(params.DefaultSuffix);

    if(params.Mode == DescImportExportSourceParams::Save){
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    }
    fileDialog.selectFile(params.FileName);
    fileDialog.setFileMode(params.Mode == DescImportExportSourceParams::Save ? QFileDialog::AnyFile :
                                          DescImportExportSourceParams::Load ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);

    if(fileDialog.exec() == QDialog::Rejected) {
        return {};
    }

    internalSettings.setValue(searchDir, fileDialog.directoryUrl().path());
    return fileDialog.selectedUrls();
}
