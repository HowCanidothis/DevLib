#include "widgetsdialogsmanager.h"

#include <QGraphicsDropShadowEffect>
#include <QBoxLayout>
#include <QApplication>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include "WidgetsModule/Bars/menubarmovepane.h"
#include "WidgetsModule/Attachments/windowresizeattachment.h"

WidgetsDialogsManager::WidgetsDialogsManager()
    : m_defaultParent(nullptr)
{

}

bool WidgetsDialogsManager::ShowOkCancelDialog(const QString& label, const QString& text)
{
    QMessageBox dialog(GetParentWindow());
    dialog.setWindowTitle(label);
    dialog.setText(text);
    auto* defaultButton = dialog.addButton(tr("Ok"), QMessageBox::YesRole);
    dialog.addButton(tr("Cancel"), QMessageBox::NoRole);
    dialog.setDefaultButton(defaultButton);
    OnDialogCreated(&dialog);
    auto res = dialog.exec();
    return res == 0;
}

void WidgetsDialogsManager::ShowMessageBox(QtMsgType msgType, const QString& title, const QString& message)
{
    QMessageBox dialog(GetParentWindow());
    dialog.setWindowTitle(title);
    dialog.setText(message);
    auto* defaultButton = dialog.addButton(tr("OK"), QMessageBox::YesRole);
    dialog.setDefaultButton(defaultButton);
    dialog.setModal(true);
    OnDialogCreated(&dialog);
    dialog.exec();
}

void WidgetsDialogsManager::SetDefaultParentWindow(QWidget* window)
{
    m_defaultParent = window;
}

QWidget* WidgetsDialogsManager::GetParentWindow() const
{
    return qApp->activeWindow() == nullptr ? m_defaultParent : qApp->activeWindow();
}

QList<QUrl> WidgetsDialogsManager::SelectDirectory(const DescImportExportSourceParams& params){
    QString searchDir(QString("last%1Folder").arg(params.Mode == DescImportExportSourceParams::Save ? "Save" : "Load"));
    QSettings internalSettings;
    auto lastSearchFolder = internalSettings.value(searchDir, QCoreApplication::applicationDirPath()).toString();
    QFileDialog fileDialog(GetParentWindow(), params.Label, lastSearchFolder);
    OnDialogCreated(&fileDialog);
    fileDialog.setNameFilters(params.Filters);
    fileDialog.setFileMode(params.Mode == DescImportExportSourceParams::Save ? QFileDialog::AnyFile :
                                          DescImportExportSourceParams::Load ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);

    if(params.Mode == DescImportExportSourceParams::Save){
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    }
    fileDialog.selectFile(params.FileName);
    fileDialog.setDefaultSuffix(params.DefaultSuffix);

    if(fileDialog.exec() == QDialog::Rejected) {
        return {};
    }

    internalSettings.setValue(searchDir, fileDialog.directoryUrl().path());
    auto result = fileDialog.selectedUrls();
    // Qt / Windows bug fix, Native File picker doesn't work correctly if File Extensions is off at the Explorer.exe
    if(params.Mode == DescImportExportSourceParams::Save && !result.isEmpty()) {
        QFileInfo fileInfo(result.first().toLocalFile());
        auto parts = fileInfo.completeSuffix().split(".");
        qint32 expectedRoots = params.ExpectedExtensionsCount;
        if(parts.size() == expectedRoots) {
            return result;
        }
        auto baseName = fileInfo.absoluteFilePath();
        baseName.remove(fileInfo.completeSuffix());
        QStringList qtBugFixRoots;
        for(const auto& part : adapters::reverse(parts)) {
            if(expectedRoots-- == 0) {
                break;
            }
            qtBugFixRoots.prepend(part);
        }
        auto url = QUrl::fromLocalFile(baseName + qtBugFixRoots.join('.'));
        return { url };
    }


    return result;
}

void WidgetsDialogsManager::MakeFrameless(QWidget* widget, bool attachMovePane)
{
    bool resizeable = true;

    if(widget->layout() != nullptr) { // TODO. It's a crutch
        if(widget->layout()->sizeConstraint() == QLayout::SetFixedSize || widget->layout()->sizeConstraint() == QLayout::SetNoConstraint) {
            resizeable = false;
        }
    }

    auto* vboxla = new QBoxLayout(QBoxLayout::TopToBottom);
    vboxla->setMargin(0);
    vboxla->setSpacing(0);
    MenuBarMovePane* pane = nullptr;
    if(attachMovePane) {
        pane = new MenuBarMovePane(widget);
        vboxla->addWidget(pane);
    }
    auto* layout = widget->layout();
    QWidget* newWidget = new QWidget();
    newWidget->setMinimumSize(widget->minimumSizeHint());
    if(!resizeable) {
        layout->setSizeConstraint(QLayout::SetNoConstraint);
    }
    newWidget->setLayout(layout);
    vboxla->addWidget(newWidget);
    widget->setLayout(vboxla);
    widget->setWindowFlag(Qt::FramelessWindowHint);

    AttachShadow(widget);

    if(pane != nullptr) {
        pane->Resizeable = resizeable;
        pane->Modal = widget->isModal();
    }
    if(resizeable) {
        WindowResizeAttachment::Attach(widget);
    }
}

void WidgetsDialogsManager::AttachShadow(QWidget* widget)
{
    auto* shadow = new QGraphicsDropShadowEffect();

    shadow->setColor(WidgetsDialogsManager::GetInstance().ShadowColor);
    shadow->setBlurRadius(WidgetsDialogsManager::GetInstance().ShadowBlurRadius);
    shadow->setOffset(0);
    widget->setGraphicsEffect(shadow);
    widget->window()->setAttribute(Qt::WA_TranslucentBackground);
    widget->window()->layout()->setMargin(10);

    DispatcherConnectionsSafe connections;

    auto qtBugFix = [widget]{
        auto widgetOldSize = widget->size();
        widget->resize(widgetOldSize.width() - 1, widgetOldSize.height());
        ThreadsBase::DoMain([widget, widgetOldSize]{
            widget->resize(widgetOldSize);
        });
    };

    WidgetsDialogsManager::GetInstance().ShadowBlurRadius.OnChange.Connect(nullptr, [shadow, qtBugFix]{
        shadow->setBlurRadius(WidgetsDialogsManager::GetInstance().ShadowBlurRadius);
        qtBugFix();
    }).MakeSafe(connections);
    WidgetsDialogsManager::GetInstance().ShadowColor.OnChange.Connect(nullptr, [shadow, qtBugFix]{
        shadow->setColor(WidgetsDialogsManager::GetInstance().ShadowColor);
        qtBugFix();
    }).MakeSafe(connections);

    QObject::connect(widget, &QWidget::destroyed, [connections]{});
}
