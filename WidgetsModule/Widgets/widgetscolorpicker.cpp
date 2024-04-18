#include "widgetscolorpicker.h"
#include "ui_widgetscolorpicker.h"

#include <QMouseEvent>
#include <QColorDialog>
#include <QSettings>
#include <QPainterPath>

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
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), m_borderRadius.width(), m_borderRadius.height());
    painter.setPen(Color);
    painter.fillPath(path, Color.Native());
    painter.drawPath(path);
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
