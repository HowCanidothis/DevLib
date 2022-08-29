#ifndef TOPNOTIFIERFRAME_H
#define TOPNOTIFIERFRAME_H

#include <QFrame>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class TopNotifierFrame;
}

class TopNotifierFrame : public QFrame
{
    Q_OBJECT
    using Super = QFrame;

public:
    explicit TopNotifierFrame(QWidget* parent);
    ~TopNotifierFrame();

    void SetText(const FTranslationHandler& text);
    void SetActionText(const FTranslationHandler& text);
    void SetAction(const FAction& action);

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Ui::TopNotifierFrame *ui;
};

class TopNotifierFrameErrorsComponent : public QObject
{
    using Super = QObject;
public:
    TopNotifierFrameErrorsComponent(LocalPropertyErrorsContainer* errors, TopNotifierFrame* frame);

private:
    DelayedCallObject m_updateText;
    DispatcherConnectionsSafe m_connections;
};

#endif // TOPNOTIFIERWIDGET_H
