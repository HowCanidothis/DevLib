#ifndef WINDOWRESIZEATTACHMENT_H
#define WINDOWRESIZEATTACHMENT_H

#include <QPoint>
#include <QObject>

class WindowResizeAttachment : public QObject
{
    WindowResizeAttachment();
public:
    static void Attach(class QWidget* widget);
    static void AttachRecursive(QWidget * widget);
    
    // QObject interface
public:
    bool eventFilter(QObject * watched, QEvent * event);
    
private:
    static WindowResizeAttachment& instance();
    
    bool m_startDrag;
    int m_borderWidth;
    QPoint m_startPos;
    Qt::MouseButton m_activeButton;
    Qt::WindowFrameSection m_cursorSection;
};

#endif // WINDOWRESIZEATTACHMENT_H
