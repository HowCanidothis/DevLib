#ifndef IGNOREWHEELWITHOUTFOCUSATTACHMENT_H
#define IGNOREWHEELWITHOUTFOCUSATTACHMENT_H

#include <QObject>

class IgnoreWheelWithoutFocusAttachment : public QObject
{
    IgnoreWheelWithoutFocusAttachment();
public:
    static void Attach(class QWidget* widget);
    static void AttachRecursive(QWidget* widget, const std::function<bool (QWidget* w)>& filter = [](QWidget*){ return true; });
    static void AttachRecursiveSpinBoxesAndComboBoxes(QWidget* widget);

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    static IgnoreWheelWithoutFocusAttachment& getInstance();
};

#endif // IGNOREWHEELWITHOUTFOCUSATTACHMENT_H
