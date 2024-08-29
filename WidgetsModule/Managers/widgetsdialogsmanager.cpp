#include "widgetsdialogsmanager.h"

#include <QGraphicsDropShadowEffect>
#include <QBoxLayout>
#include <QApplication>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QDesktopWidget>
#include <QSettings>
#include <QInputDialog>

#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/Bars/menubarmovepane.h"
#include "WidgetsModule/Attachments/windowresizeattachment.h"
#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Dialogs/widgetsinputdialog.h"

const char* WidgetsDialogsManager::CustomViewPropertyKey = "CustomView";

WidgetsDialogsManager::WidgetsDialogsManager()
    : m_defaultParent(nullptr)
{

}

bool WidgetsDialogsManager::ShowOkCancelDialog(const QString& label, const QString& text, const QString& confirmActionText)
{
    QMessageBox dialog(GetParentWindow());
//    dialog.setWindowTitle(label);
    dialog.setText(QString(R"(<p style="font-size:24px;">%1</p>%2.)").arg(label, text));
    auto* defaultButton = dialog.addButton(confirmActionText.isEmpty() ? tr("Confirm") : confirmActionText, QMessageBox::YesRole);
    dialog.addButton(tr("Cancel"), QMessageBox::NoRole);
    dialog.setDefaultButton(defaultButton);
    OnDialogCreated(&dialog);
    auto res = dialog.exec();
    return res == 0;
}

void WidgetsDialogsManager::ShowMessageBox(QtMsgType , const QString& title, const QString& text)
{
    QMessageBox dialog(GetParentWindow());
//    dialog.setWindowTitle(title);
    dialog.setText(QString(R"(<p style="font-size:24px;">%1</p>%2.)").arg(title, text));
    auto* defaultButton = dialog.addButton(tr("Ok"), QMessageBox::YesRole);
    dialog.setDefaultButton(defaultButton);
    dialog.setModal(true);
    OnDialogCreated(&dialog);
    dialog.exec();
}

QString WidgetsDialogsManager::GetText(const QString& title, const QString& text, bool* ok)
{
    LocalPropertyString v(text);
    WidgetsInputDialog dialog(GetParentWindow());
    dialog.AddLineText(title, &v);

    OnDialogCreated(&dialog);

    const int ret = dialog.exec();
    if (ok)
        *ok = !!ret;
    if (ret) {
        return v;
    } else {
        return QString();
    }
}

QDialog* WidgetsDialogsManager::GetOrCreateCustomDialog(const Name& tag, const std::function<DescCustomDialogParams ()>& paramsCreator)
{
    return GetOrCreateDialog<QDialog>(tag, [this, paramsCreator]{
        auto params = paramsCreator();
        auto* dialog = new QDialog(GetParentWindow());
        auto* vlayout = new QVBoxLayout();
        dialog->setLayout(vlayout);
        vlayout->addWidget(params.View);
        dialog->setProperty(CustomViewPropertyKey, (size_t)params.View);
        if(!params.DefaultSpacing) {
            vlayout->setContentsMargins(0,0,0,0);
            vlayout->setSpacing(0);
        }

        if(params.Buttons.isEmpty()) {
            return dialog;
        }

        auto* buttons = new QDialogButtonBox();
        vlayout->addWidget(buttons);

        QObject::connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
        QObject::connect(dialog, &QDialog::rejected, params.OnRejected);
        QObject::connect(dialog, &QDialog::accepted, params.OnAccepted);

        qint32 index = 0;
        for(const auto& buttonParams : params.Buttons) {
            const auto& [role, text, inaction] = buttonParams;
            FAction action = inaction;
            auto* button = buttons->addButton(text, role);
            if(index == params.DefaultButtonIndex) {
                button->setDefault(true);
            }
            QObject::connect(button, &QPushButton::clicked, [action]{
                action();
            });
            index++;
        }
        return dialog;
    });
}

void WidgetsDialogsManager::ShowDialog(QDialog* dialog, const DescShowDialogParams& params)
{
    dialog->setParent(GetParentWindow(), dialog->windowFlags());

    if(params.ResizeToDefault) {
        ResizeDialogToDefaults(dialog);
    }

    dialog->setModal(params.Modal);

    if(params.Modal) {
        dialog->exec();
    } else {
        dialog->show();
    }
}

void WidgetsDialogsManager::ShowPropertiesDialog(const PropertiesScopeName& scope, const DescShowDialogParams& params)
{
    auto* dialog = GetOrCreateDialog<PropertiesDialog>(scope, [this, scope]{
        return new PropertiesDialog(scope, GetParentWindow());
    }, scope);
    dialog->Initialize([]{});
    dialog->GetView<PropertiesView>()->expandAll();
    ShowDialog(dialog, params);
}

void WidgetsDialogsManager::ResizeDialogToDefaults(QWidget* dialog)
{
    auto* desktop = QApplication::desktop();
    auto size = desktop->screenGeometry(dialog);
    dialog->resize(2 * size.width() / 3, 2 * size.height() / 3);
}

void WidgetsDialogsManager::SetDefaultParentWindow(QWidget* window)
{
    m_defaultParent = window;
}

QWidget* WidgetsDialogsManager::GetParentWindow() const
{
    return qApp->activeWindow() == nullptr ? m_defaultParent : qApp->activeWindow();
}

bool WidgetsDialogsManager::ShowSaveCancelDialog(const QString& label, const QString& text)
{
    QMessageBox dialog(GetParentWindow());
//    dialog.setWindowTitle(label);
    dialog.setText(QString(R"(<p style="font-size:24px;">%1</p>%2.)").arg(label, text));
    auto* defaultButton = dialog.addButton(tr("Save"), QMessageBox::YesRole);
    dialog.addButton(tr("Cancel"), QMessageBox::NoRole);
    dialog.setDefaultButton(defaultButton);
    OnDialogCreated(&dialog);
    auto res = dialog.exec();
    return res == 0;
}

bool WidgetsDialogsManager::ShowDeleteCancelDialog(const QString& label, const QString& text)
{
    QMessageBox dialog(GetParentWindow());
//    dialog.setWindowTitle(label);
    dialog.setText(QString(R"(<p style="font-size:24px;">%1</p>%2.)").arg(label, text));
    auto* defaultButton = dialog.addButton(tr("Delete"), QMessageBox::DestructiveRole);
    dialog.addButton(tr("Cancel"), QMessageBox::NoRole);
    dialog.setDefaultButton(defaultButton);
    OnDialogCreated(&dialog);
    auto res = dialog.exec();
    return res == 0;
}

QList<QUrl> WidgetsDialogsManager::SelectDirectory(const DescImportExportSourceParams& params){
    QString searchDir(QString("last%1Folder").arg(params.Mode == DescImportExportSourceParams::Save ? "Save" : "Load"));
    QSettings internalSettings;
    auto lastSearchFolder = internalSettings.value(searchDir, QCoreApplication::applicationDirPath()).toString();
    QFileDialog fileDialog(GetParentWindow(), params.Label, lastSearchFolder);
    OnDialogCreated(&fileDialog);
    auto filters = params.Filters;
    if(params.Mode == DescImportExportSourceParams::Save) {
        std::sort(filters.begin(), filters.end(), [&](const QString& f, const QString& s){
            auto fcontains = f.contains(params.DefaultSuffix);
            auto scontains = s.contains(params.DefaultSuffix);
            if(fcontains != scontains) {
                return fcontains;
            }
            return f.size() < s.size();
        });
    }
    fileDialog.setNameFilters(filters);
    fileDialog.setFileMode(params.Mode == DescImportExportSourceParams::Save ? QFileDialog::AnyFile :
                                          DescImportExportSourceParams::Load ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile);

    if(params.Mode == DescImportExportSourceParams::Save){
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    }
    fileDialog.selectFile(params.FileName);

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
#include <QResizeEvent>
void WidgetsDialogsManager::MakeFrameless(QWidget* widget, bool attachMovePane, const QString& movePaneId)
{
    if(widget->layout() == nullptr) {
        return;
    }

    bool resizeable = true;

    if(widget->layout()->sizeConstraint() == QLayout::SetFixedSize || widget->layout()->sizeConstraint() == QLayout::SetNoConstraint) {
        resizeable = false;
    }

    auto createNullLayout = []{
        auto* result = new QVBoxLayout();
        result->setMargin(0);
        result->setSpacing(0);
        return result;
    };

    QVBoxLayout* contentWithPaneWidgetLayout = createNullLayout();
    MenuBarMovePane* pane = nullptr;
    if(attachMovePane) {
        pane = new MenuBarMovePane(widget);
        pane->setProperty("a_type", movePaneId);
        contentWithPaneWidgetLayout->addWidget(pane);
    }
    QWidget* contentWithPaneWidget = new QWidget();
    QWidget* contentWidget = new QWidget();
    contentWithPaneWidgetLayout->addWidget(contentWidget);
    contentWithPaneWidget->setLayout(contentWithPaneWidgetLayout);


    auto* layout = widget->layout();
    widget->window()->setWindowFlag(Qt::FramelessWindowHint);
    widget->window()->setAttribute(Qt::WA_TranslucentBackground);

    widget->show();
    layout->invalidate();
    widget->hide();

    contentWidget->setLayout(layout);
    AttachShadow(contentWithPaneWidget, false);

    if(!resizeable) {
        contentWithPaneWidget->setMinimumSize(widget->size() + QSize(20,20));
        layout->setSizeConstraint(QLayout::SetNoConstraint);
    } else {
        contentWithPaneWidget->setMinimumSize(widget->minimumSizeHint() + QSize(20,20));
    }
    auto* mainLayout = createNullLayout();
    mainLayout->addWidget(contentWithPaneWidget);
    widget->setLayout(mainLayout);

    widget->window()->layout()->setMargin(10);

    if(pane != nullptr) {
        pane->Resizeable = resizeable;
        WidgetWrapper(widget).AddEventFilter([widget, pane](QObject*, QEvent* e){
            if(e->type() == QEvent::Show) {
                pane->Modal = widget->isModal();
            }
            return false;
        });
    }
    if(resizeable) {
        WindowResizeAttachment::Attach(widget);
    }
}

void WidgetsDialogsManager::AttachShadow(QWidget* widget, bool applyMargins)
{
    auto* shadow = new QGraphicsDropShadowEffect();

    shadow->setColor(WidgetsDialogsManager::GetInstance().ShadowColor);
    shadow->setBlurRadius(WidgetsDialogsManager::GetInstance().ShadowBlurRadius);
    shadow->setOffset(0);
    widget->setGraphicsEffect(shadow);

    auto connections = WidgetWrapper(widget).WidgetConnections();

    WidgetsDialogsManager::GetInstance().ShadowBlurRadius.Connect(CDL, [shadow](auto radius){
        shadow->setBlurRadius(radius);
    }).MakeSafe(connections);
    WidgetsDialogsManager::GetInstance().ShadowColor.Connect(CDL, [shadow](const auto& color){
        shadow->setColor(color);
    }).MakeSafe(connections);
}
