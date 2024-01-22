#ifndef WIDGETSCOLORPICKER_H
#define WIDGETSCOLORPICKER_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsColorPicker;
}

class WidgetsColorPicker : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetsColorPicker(QWidget *parent = nullptr);
    ~WidgetsColorPicker();

    LocalPropertyColor Color;

private:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::WidgetsColorPicker *ui;
};

#endif // WIDGETSCOLORPICKER_H
