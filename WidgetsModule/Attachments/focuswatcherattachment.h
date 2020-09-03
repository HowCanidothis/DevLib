#ifndef FOCUSWATCHERATTACHMENT_H
#define FOCUSWATCHERATTACHMENT_H

#include <SharedModule/internal.hpp>

class FocusManager // TODO. Maybe not static
{
    FocusManager(){}
public:
    static FocusManager& GetInstance();

    CommonDispatcher<QWidget*, bool> OnFocusChanged;
};

class FocusWatcherAttachment : public QObject
{
    FocusWatcherAttachment(QWidget* target);
public:
    static void Attach(QWidget* widget);

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget* m_target;
    bool m_hasFocus;
};

#endif // FOCUSWATCHERATTACHMENT_H
