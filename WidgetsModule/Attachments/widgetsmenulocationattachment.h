#ifndef WIDGETSMENULOCATIONATTACHMENT_H
#define WIDGETSMENULOCATIONATTACHMENT_H

#include <SharedModule/internal.hpp>

class WidgetsMenuLocationAttachment : public QObject
{
public:

    static void Attach(class QMenu* menu, QuadTreeF::BoundingRect_Location location);

private:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    static WidgetsMenuLocationAttachment& getInstance();
};

#endif // WIDGETSMENULOCATIONATTACHMENT_H
