#ifndef WIDGETSTOOLBAR_H
#define WIDGETSTOOLBAR_H

#include <QFrame>
#include <PropertiesModule/internal.hpp>

class WidgetsToolBar : public QFrame
{
    using Super = QFrame;
public:
    struct ButtonInfo {
        QString ObjectName;
        std::function<void(bool)> Handler;
    };
    WidgetsToolBar(QWidget* parent, Qt::Orientation orientation = Qt::Vertical);
    void SetTarget(QWidget* target, QuadTreeF::BoundingRect_Location location);

    class ComponentPlacer* GetComponentPlacer() const;

    class QPushButton* CreateDrawerButton(QWidget* drawer, qint32 drawerSize = 450);
    QPushButton* CreateButton(const QString& objectName);

private:
    class QBoxLayout* m_layout;
};

#endif // WidgetsToolBar_H
