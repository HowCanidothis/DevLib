#ifndef MENUBARMOVEPANE_H
#define MENUBARMOVEPANE_H

#include <QFrame>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class MenuBarMovePane;
}

class MenuBarMovePane : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
public:
    MenuBarMovePane(QWidget* parent = nullptr);
    
    LocalPropertyBool Closeable;
    LocalPropertyBool Resizeable;
    LocalPropertyBool Modal;
    CommonDispatcher<QWidget*> OnGeometryRestored;

    void SetWindow(QWidget* w);

    class QHBoxLayout* GetPlaceHolder() const;
    QString GetTitle() const;
    QLabel* GetTitleWidget() const;
    void SetTitle(const QString& title);
    void MaximizeRestore();

    // QObject interface
public:
    bool filter(QObject* watched, QEvent* event);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    void on_BtnMinimize_clicked();
    void on_BtnMaximize_clicked();
    void on_BtnClose_clicked();

private:
    Ui::MenuBarMovePane* ui;
    class EventFilterObject* m_windowEventFilter;
    QByteArray m_windowGeometry;
    QPoint m_pressPoint;
    bool m_ignoreMoveEvents;
    bool m_resizeable;

#ifdef Q_OS_WIN
    bool m_isDrag;
#endif
};

#endif // MENUBARMOVEPANE_H
