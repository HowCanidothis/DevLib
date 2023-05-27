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

    TranslatedString& WidgetText();
    LocalProperty<QuadTreeF::BoundingRect_Location>& WidgetLocation();
    LocalProperty<QPoint>& WidgetOffset();

private:
    Ui::TopNotifierFrame *ui;
    class WidgetsLocationAttachment* m_placer;
};

class TopNotifierFrameErrorsComponent : public QObject
{
    using Super = QObject;
public:
    TopNotifierFrameErrorsComponent(TopNotifierFrame* frame);
    TopNotifierFrameErrorsComponent(LocalPropertyErrorsContainer* errors, TopNotifierFrame* frame);
    ~TopNotifierFrameErrorsComponent();

    LocalPropertyErrorsContainer& GetErrors() { return *m_errors; }

private:
    DelayedCallObject m_updateText;
    DispatcherConnectionsSafe m_connections;
    LocalPropertyErrorsContainer* m_errors;
    bool m_internalErrors;
};

#endif // TOPNOTIFIERWIDGET_H
