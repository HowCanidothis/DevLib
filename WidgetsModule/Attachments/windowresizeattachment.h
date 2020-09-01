#ifndef WINDOWRESIZEATTACHMENT_H
#define WINDOWRESIZEATTACHMENT_H

#include <QPoint>
#include <QObject>

class WindowResizeAttachment : public QObject
{
    WindowResizeAttachment();
public:
    static void Attach(class QWidget* widget);
    
    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event);
    
private:
    enum Location
    {
        Location_Default = 0x0,
        Location_Top = 0x1,
        Location_Right = 0x2,
        Location_Left = 0x4,
        Location_Bottom = 0x8,

        Location_BottomLeft = Location_Left | Location_Bottom,
        Location_BottomRight = Location_Right | Location_Bottom,
        Location_TopLeft = Location_Left | Location_Top,
        Location_TopRight = Location_Right | Location_Top
    };

private:
    void resize(QWindow* window, const QPoint& pos, Location location);
    void installCursorFromLocation(QWindow* window, Location location);
    Location findLocation(QWindow* window, const QPoint& pos);
    static WindowResizeAttachment& instance();

    int m_borderWidth;
    Location m_draggingLocation;
    Location m_previousMoveLocation;
};

#endif // WINDOWRESIZEATTACHMENT_H
