#include "widgetshyperlinkedlistdialogview.h"
#include "ui_widgetshyperlinkedlistdialogview.h"

#include <WidgetsModule/internal.hpp>

WidgetsHyperlinkedListDialogView::WidgetsHyperlinkedListDialogView(QWidget *parent) :
    Super(parent),
    ui(new Ui::WidgetsHyperlinkedListDialogView)
{
    ui->setupUi(this);
    ui->lbList->setTextFormat(Qt::RichText);
    ui->lbList->setOpenExternalLinks(false);

    setWindowTitle(QString());

    WidgetLabelWrapper(ui->lbList).OnLinkActivated().Connect(CONNECTION_DEBUG_LOCATION, [](const Name& link){
        (*(FAction*)link.AsString().toLongLong())();
    });
}

WidgetsHyperlinkedListDialogView::~WidgetsHyperlinkedListDialogView()
{
    delete ui;
}

void WidgetsHyperlinkedListDialogView::SetText(const QString& text, const QString& content)
{
    ui->lbText->setText(text);
    ui->lbList->setText(content);
}
