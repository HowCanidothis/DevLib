#include "widgetsdialogsmanager.h"

#include <QGraphicsDropShadowEffect>
#include <QBoxLayout>
#include <QApplication>
#include <QFileDialog>
#include <QSettings>

#include "WidgetsModule/Bars/menubarmovepane.h"
#include "WidgetsModule/Attachments/windowresizeattachment.h"

WidgetsDialogsManager::WidgetsDialogsManager()
    : m_defaultParent(nullptr)
{

}

void WidgetsDialogsManager::SetDefaultParentWindow(QWidget* window)
{
    m_defaultParent = window;
}

QWidget* WidgetsDialogsManager::GetParentWindow() const
{
    return qApp->activeWindow() == nullptr ? m_defaultParent : qApp->activeWindow();
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
    QFileDialog fileDialog(GetParentWindow(), params.Label, lastSearchFolder);
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
