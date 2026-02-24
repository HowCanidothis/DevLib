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
    ui(new Ui::WidgetsColorPicker),
    HasAlpha(false)
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

    auto currentSize = size();
    QStyleOption opt;
    auto sizeWithMargins = style()->sizeFromContents(QStyle::CT_PushButton, &opt, currentSize, this);
    ///PM_DockWidgetTitleBarButtonMargin to get top margin
    auto leftMargin = style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, &opt, this);
    auto rightMargin = sizeWithMargins.width() - currentSize.width() - leftMargin;

    QPainterPath path;
    path.addRoundedRect(rect().adjusted(leftMargin, 0, -rightMargin, 0), m_borderRadius.width(), m_borderRadius.height());
    painter.setPen(palette().text().color());
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
        auto result = WidgetsDialogsManager::GetInstance().GetColor(Color, HasAlpha);
        if(result.has_value()) {
            Color = result.value();
        }
    }
}
