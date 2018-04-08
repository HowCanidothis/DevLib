#ifndef QTCUSTOMEVENTS_H
#define QTCUSTOMEVENTS_H

#include <QEvent>
#include <functional>

class QtInlineEvent : public QEvent
{
public:
    typedef std::function<void ()> Function;
    QtInlineEvent(const Function& function)
        : QEvent(QEvent::None)
        , function(function)
    {}
    ~QtInlineEvent() {
        function();
    }

    static void post(const Function& function);
private:
    Function function;
};

#endif // QTCUSTOMEVENTS_H
