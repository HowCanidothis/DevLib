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
    Q_PROPERTY(QSize borderRadius MEMBER m_borderRadius)
public:
    explicit WidgetsColorPicker(QWidget *parent = nullptr);
    ~WidgetsColorPicker();

    LocalPropertyColor Color;
    bool HasAlpha;

private:
    void paintEvent(QPaintEvent*) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::WidgetsColorPicker *ui;
    QSize m_borderRadius;
};

#endif // WIDGETSCOLORPICKER_H
