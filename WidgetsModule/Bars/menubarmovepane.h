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
    
    LocalPropertyBool Resizeable;
    LocalPropertyBool Modal;

    QString GetTitle() const;
    void SetTitle(const QString& title);
    void MaximizeRestore();

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;

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
    QByteArray m_windowGeometry;
    QPoint m_pressPoint;
    bool m_ignoreMoveEvents;
    bool m_resizeable;

#ifdef Q_OS_WIN
    bool m_isDrag;
#endif
};

#endif // MENUBARMOVEPANE_H
