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

    bool eventFilter(QObject* watched, QEvent* event);
    static Location FindLocation(const QRectF& rect, const QPointF& pos, qint32 border);
    template<class T>
    static void InstallCursorFromLocation(T* w, Location location)
    {
        switch (location) {
        case Location_Top:
        case Location_Bottom:
            w->setCursor(Qt::SizeVerCursor);
            break;
        case Location_Left:
        case Location_Right:
            w->setCursor(Qt::SizeHorCursor);
            break;
        case Location_TopRight:
        case Location_BottomLeft:
            w->setCursor(Qt::SizeBDiagCursor);
            break;
        case Location_BottomRight:
        case Location_TopLeft:
            w->setCursor(Qt::SizeFDiagCursor);
            break;
        default:
            w->unsetCursor();
            break;
        }
    }

private:
    void resize(QWindow* window, const QPoint& pos, Location location);
    Location findLocation(QWindow* window, const QPoint& pos);
    static WindowResizeAttachment& instance();

    int m_borderWidth;
    Location m_draggingLocation;
    Location m_previousMoveLocation;
};

#endif // WINDOWRESIZEATTACHMENT_H
