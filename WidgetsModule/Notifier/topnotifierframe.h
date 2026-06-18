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

    const TranslatedStringPtr& WidgetText();
    LocalProperty<QuadTreeF::BoundingRect_Location>& WidgetLocation();
    LocalProperty<QPoint>& WidgetOffset();

private:
    Ui::TopNotifierFrame *ui;
    class WidgetsLocationAttachment* m_placer;
};

class NotifierFrame : public TopNotifierFrame
{
    using Super = TopNotifierFrame;
public:
    explicit NotifierFrame(QWidget* parent)
        : Super(parent)
    {
        WidgetLocation() = QuadTreeF::Location_Center;
    }
};

class TopNotifierFrameErrorsFocusComponent : public QObject
{
    using Super = QObject;
public:
    TopNotifierFrameErrorsFocusComponent(QObject* parent);
    ~TopNotifierFrameErrorsFocusComponent();

    template<class ... Args>
    TopNotifierFrameErrorsFocusComponent& Register(QWidget* w, const Name& id1, const Name& id2, const Args&... ids)
    {
        adapters::Combine([&](const auto& id) {
            m_map[w].insert(id);
        }, id1, id2, ids...);
        return *this;
    }
    TopNotifierFrameErrorsFocusComponent& Register(QWidget* w, const Name& id) { m_map[w] += id; return *this; }
    void FocusWidget(const Name& focusError);
    DispatcherConnections ConnectFromViewModel(const char* cdl, const LocalPropertyErrorsViewModel* models);

    CommonDispatcher<const Name&, QWidget*> OnWidgetFocused;

private:
    void updateHighlighted();
    DelayedCallObject m_updater;
    QHash<QWidget*, QSet<Name>> m_map;
    DispatcherConnectionsSafe m_modelConnections;
    CommonDispatcher<QHash<Name, TranslatedStringPtr>&> m_collectActiveErrors;
};

class TopNotifierFrameErrorsComponent : public QObject
{
    using Super = QObject;
public:
    TopNotifierFrameErrorsComponent(TopNotifierFrame* frame);
    TopNotifierFrameErrorsComponent(LocalPropertyErrorsViewModel* errors, TopNotifierFrame* frame);
    ~TopNotifierFrameErrorsComponent();

    DispatcherConnections AddFocusComponent(TopNotifierFrameErrorsFocusComponent* component);

    LocalPropertyErrorsViewModel& GetErrors() { return *m_errors; }

    CommonDispatcher<const Name&>& OnErrorActivated();

private:
    DelayedCallObject m_updateText;
    DispatcherConnectionsSafe m_connections;
    LocalPropertyErrorsViewModel* m_errors;
    bool m_internalErrors;
#ifdef QT_DEBUG
    QSet<TopNotifierFrameErrorsFocusComponent*> m_focusComponents;
#endif
    QVector<SP<FAction>> m_defaultActivationHandlers;
    bool m_useDefaultActionHandlers;
    CommonDispatcher<const Name&> m_defaultActionHandler;
};

#endif // TOPNOTIFIERWIDGET_H
