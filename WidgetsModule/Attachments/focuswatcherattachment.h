#ifndef FOCUSWATCHERATTACHMENT_H
#define FOCUSWATCHERATTACHMENT_H

#include <SharedModule/internal.hpp>
#include <PropertiesModule/internal.hpp>

class FocusManager // TODO. Maybe not static
{
    friend class FocusWatcherAttachment;
    FocusManager();

    void destroyed(QWidget* target);
    void focusGot(QWidget* target);

public:
    static FocusManager& GetInstance();

    void ResetFocus();
    QWidget* GetPreviousFocusedWidget() const { return m_previousFocusedWidget; }
    const LocalProperty<QWidget*>& FocusedWidget() const { return m_focusedWidget; }

private:
    QWidget* m_previousFocusedWidget;
    LocalProperty<QWidget*> m_focusedWidget;
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
};

#endif // FOCUSWATCHERATTACHMENT_H
