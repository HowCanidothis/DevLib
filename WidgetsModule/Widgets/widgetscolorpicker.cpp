#include "widgetscolorpicker.h"
#include "ui_widgetscolorpicker.h"

#include <QMouseEvent>
#include <QColorDialog>
#include <QSettings>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Managers/widgetsdialogsmanager.h"

WidgetsColorPicker::WidgetsColorPicker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetsColorPicker)
{
    ui->setupUi(this);

    Color.OnChanged.Connect(CDL, [this]{
        update();
    });
}

void WidgetsColorPicker::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setPen(Color);
    painter.fillRect(rect(), Color);
    painter.drawRect(rect());
}

WidgetsColorPicker::~WidgetsColorPicker()
{
    delete ui;
}

void WidgetsColorPicker::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) {
        WidgetColorDialogWrapper dialog(WidgetsDialogsManager::GetInstance().GetOrCreateDialog<QColorDialog>("ColorDialog", []{
            return WidgetColorDialogWrapper(new QColorDialog(WidgetsDialogsManager::GetInstance().GetParentWindow()))
                   .SetDefaultLabels();
        }, "ColorDialog"));
        dialog->setCurrentColor(Color);
        dialog.Show(DescShowDialogParams());
        if(dialog->result() == QDialog::Accepted) {
            auto result = dialog->currentColor();
            Color = result;
        }
    }
}
