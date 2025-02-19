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
#include <QColorDialog>

#include <PropertiesModule/internal.hpp>

#include "WidgetsModule/Bars/menubarmovepane.h"
#include "WidgetsModule/Attachments/windowresizeattachment.h"
#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Dialogs/widgetsinputdialogview.h"
#include "WidgetsModule/Dialogs/widgetsdialog.h"

const char* WidgetsDialogsManager::CustomViewPropertyKey = "CustomView";
const char* WidgetsDialogsManager::FDialogHandlerPropertyName = "DialogHandler";
const char* WidgetsDialogsManager::ResizeablePropertyName = "Resizeable";

WidgetsDialogsManager::WidgetsDialogsManager()
    : m_defaultParent(nullptr)
{

}

WidgetsDialog* WidgetsDialogsManager::createDialog(const DescCustomDialogParams& params) const
{
    WidgetsDialog* dialog = new WidgetsDialog(GetParentWindow());
    if(params.Title != nullptr) {
        dialog->SetHeaderText(params.Title);
    }
    auto* view = params.View;
    const auto& buttons = params.Buttons;
    if(view != nullptr) {
        dialog->SetContent(view);
        dialog->setFocusProxy(view);
        auto handler = view->property("DialogHandler");
        if(handler.isValid()) {
            (*((FDialogHandler*)handler.toLongLong()))(dialog);
        }
    }

    for(const auto& b : buttons) {
        dialog->AddButton(b);
    }
    if(params.Resizeable.has_value()) {
        dialog->setProperty(ResizeablePropertyName, params.Resizeable.value());
    }
    dialog->Initialize(params.OnDone, params.OnInitialized);
    return dialog;
}

void WidgetsDialogsManager::ShowTextDialog(const QString& title, const QString& text) const
{
    ShowTempDialog(DescCustomDialogParams().SetTitle(TR(title, =)).AddButton(WidgetsDialogsManagerDefaultButtons::CloseButton()).FillWithText(text));
}

bool WidgetsDialogsManager::ShowDeleteCancelDialog(const QString& title, const QString& text)
{
    return WidgetsDialogsManager::GetInstance().ShowTempDialog(DescCustomDialogParams()
                                                        .SetTitle(TR(title, =))
                                                        .FillWithText(text)
                                                        .AddButtons(WidgetsDialogsManagerDefaultButtons::CancelButton(),
                                                                   WidgetsDialogsManagerDefaultButtons::DeleteButton()));
}

qint32 WidgetsDialogsManager::ShowTempDialog(const DescCustomDialogParams& params, const DescShowDialogParams& showParams) const
{
    ScopedPointer<WidgetsDialog> dialog(createDialog(params));
    OnDialogCreated(dialog.get());
    DescShowDialogParams copy = showParams;
    copy.SetModal(true);
    auto result = ShowDialog(dialog.get(), copy);
    showParams.OnResult(result);
    return result;
}

std::optional<QString> WidgetsDialogsManager::GetText(const FTranslationHandler& title, const QString& text, const QStringList& keys)
{
    LocalPropertyString v(text);
    auto* dialogView = new WidgetsInputDialogView();
    dialogView->AddLineText(title(), &v, keys);

    auto res = ShowTempDialog(DescCustomDialogParams().SetTitle(TR(tr("Input Text:"))).SetView(dialogView)
        .AddButtons(WidgetsDialogsManagerDefaultButtons::CancelButton(),
                    WidgetsDialogsManagerDefaultButtons::ConfirmButton())
        .SetOnDone([&](qint32 v) {
        if(v == 0) {
            dialogView->Reset();
        }
    }));
    if(res == 0) {
        return std::nullopt;
    }

    return v;
}

std::optional<QDate> WidgetsDialogsManager::GetDate(const FTranslationHandler& title, const QDate& current)
{
    LocalPropertyDate v(current.isValid() ? current : QDate::currentDate());
    auto* dialogView = new WidgetsInputDialogView();
    dialogView->AddDate(title(), &v);

    auto res = ShowTempDialog(DescCustomDialogParams().SetTitle(TR(tr("Select Date:"))).SetView(dialogView)
        .AddButtons(WidgetsDialogsManagerDefaultButtons::CancelButton(),
                    WidgetsDialogsManagerDefaultButtons::ConfirmButton())
        .SetOnDone([&](qint32 v) {
        if(v == 0) {
            dialogView->Reset();
        }
    }));
    if(res == 0) {
        return std::nullopt;
    }

    return v;
}

std::optional<std::pair<QDate, QDate>> WidgetsDialogsManager::GetDateRange(const FTranslationHandler& title, const QDate& from, const QDate& to)
{
    LocalPropertyDate fromP(from.isValid() ? from : QDate::currentDate()), toP(to.isValid() ? to : QDate::currentDate());
    fromP.SetMinMax(from, to);
    toP.SetMinMax(from, to);
    auto* dialogView = new WidgetsInputDialogView();
    dialogView->AddDateRange(title(), &fromP, &toP);
    auto res = ShowTempDialog(DescCustomDialogParams().SetTitle(title).SetView(dialogView)
        .AddButtons(WidgetsDialogsManagerDefaultButtons::CancelButton(),
                    WidgetsDialogsManagerDefaultButtons::ConfirmButton())
        .SetOnDone([&](qint32 v) {
        if(v == 0) {
            dialogView->Reset();
        }
    }));
    if(res == 0) {
        return std::nullopt;
    }

    return std::make_pair(fromP.Native(), toP.Native());
}

std::optional<QColor> WidgetsDialogsManager::GetColor(const QColor& color, bool showAlpha)
{
    static std::optional<QColor> result;
    result = std::nullopt;
    auto createParams = [&](const WidgetColorDialogWrapper& wrapper) {
        return DescCustomDialogParams()
        .SetTitle(TR(tr("Apply Color?")))
        .SetView(wrapper.SetDefaultLabels())
        .AddButtons(WidgetsDialogsManagerDefaultButtons::CancelButton(),
                    WidgetsDialogsManagerDefaultButtons::DiscardRoleButton(TR("Remove Color")),
            WidgetsDialogsManagerDefaultButtons::ApplyButton())
        .SetOnDone([wrapper](qint32 r) {
            switch(r) {
            case 1:
                result = Qt::transparent; break;
            case 2: {
                auto cColor = wrapper->currentColor();
                if(cColor.alpha() == 0) {
                    if(cColor.rgb() != 0) {
                        cColor.setAlpha(255);
                    }
                }
                result = cColor; break;
            }
            default: break;
            }
        });
    };
    auto* dialog = GetOrCreateDialog("ColorDialog", [createParams]{
        auto* dialog = new QColorDialog();
//        dialog->layout()->setContentsMargins(0,0,0,0);
        return createParams(dialog);
    });
    auto* view = dialog->GetView<QColorDialog>();
    view->setOption(QColorDialog::ShowAlphaChannel, showAlpha);
    view->setCurrentColor(color);
    ShowDialog(dialog);

    return result;
}

WidgetsDialog* WidgetsDialogsManager::GetOrCreateDialog(const Name& tag, const std::function<DescCustomDialogParams ()>& paramsCreator)
{
    return GetOrCreateDialog<WidgetsDialog>(tag, [this, paramsCreator]{
        return createDialog(paramsCreator());
    });
}

qint32 WidgetsDialogsManager::ShowDialog(WidgetsDialog* dialog, const DescShowDialogParams& params) const
{
    dialog->setParent(GetParentWindow(), dialog->windowFlags());

    if(params.ResizeToDefault) {
        ResizeDialogToDefaults(dialog);
    } else {
        dialog->setMinimumWidth(400);
    }

    dialog->setModal(params.Modal);
    dialog->setFocus();

    qint32 result = -1;
    if(params.Modal) {
        result = dialog->exec();
    } else {
        dialog->show();
    }
    return result;
}

void WidgetsDialogsManager::ResizeDialogToDefaults(QWidget* dialog) const
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

QList<QUrl> WidgetsDialogsManager::SelectDirectory(const DescImportExportSourceParams& params){
    QString searchDir(QString("last%1Folder").arg(params.Mode == DescImportExportSourceParams::Save ? "Save" : "Load"));
    QSettings internalSettings;
    QString lastSearchFolder;
    if(params.DefaultDirectory.isEmpty()) {
        lastSearchFolder = internalSettings.value(searchDir, QCoreApplication::applicationDirPath()).toString();
    } else {
        lastSearchFolder = params.DefaultDirectory;
    }
    QFileDialog fileDialog(GetParentWindow(), QString(), lastSearchFolder);
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

void WidgetsDialogsManager::MakeFrameless(QWidget* widget, bool attachMovePane, const QString& movePaneId)
{
    if(widget->layout() == nullptr) {
        return;
    }

    auto resizeable = widget->property(ResizeablePropertyName).toBool();

    auto createNullLayout = []{
        auto* result = new QVBoxLayout();
        result->setMargin(0);
        result->setSpacing(0);
        return result;
    };

    auto size = widget->layout()->sizeHint();

    QVBoxLayout* contentWithPaneWidgetLayout = createNullLayout();
    MenuBarMovePane* pane = nullptr;
    if(attachMovePane) {
        pane = new MenuBarMovePane(widget);
        pane->setProperty("a_type", movePaneId);
        contentWithPaneWidgetLayout->addWidget(pane);
        size.rheight() += pane->sizeHint().height();
    }
    QWidget* contentWithPaneWidget = new QWidget();
    QWidget* contentWidget = new QWidget();
    contentWithPaneWidgetLayout->addWidget(contentWidget);
    contentWithPaneWidget->setLayout(contentWithPaneWidgetLayout);

    auto* layout = widget->layout();
    auto cm = layout->contentsMargins();
    cm.setTop(0);
    layout->setContentsMargins(cm);
    widget->window()->setWindowFlag(Qt::FramelessWindowHint);
    widget->window()->setAttribute(Qt::WA_TranslucentBackground);

    contentWidget->setLayout(layout);

    AttachShadow(contentWithPaneWidget, false);

    auto* mainLayout = createNullLayout();
    mainLayout->addWidget(contentWithPaneWidget);
    widget->setLayout(mainLayout);
    widget->setMinimumSize(size + QSize(20,20));

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
