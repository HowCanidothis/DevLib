#ifndef TOPNOTIFIERFRAME_H
#define TOPNOTIFIERFRAME_H

#include <QFrame>

namespace Ui {
class TopNotifierFrame;
}

struct WidgetAppearance
{
    static void SetVisibleAnimated(QWidget* widget, bool visible);
    static void ShowAnimated(QWidget* widget);
    static void HideAnimated(QWidget* widget);
};

class TopNotifierFrame : public QFrame
{
    Q_OBJECT
    using Super = QFrame;

public:
    explicit TopNotifierFrame(QWidget* parent);
    ~TopNotifierFrame();

    void SetText(const QString& text);
    void SetActionText(const QString& text);
    void SetAction(const FAction& action);

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Ui::TopNotifierFrame *ui;
};

#endif // TOPNOTIFIERWIDGET_H
