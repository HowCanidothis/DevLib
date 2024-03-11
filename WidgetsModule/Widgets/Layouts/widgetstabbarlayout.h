#ifndef WIDGETSTABBARLAYOUT_H
#define WIDGETSTABBARLAYOUT_H

#include <QFrame>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class WidgetsTabBarLayout;
}

class WidgetsTabBarLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qint32 currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(qint32 gap READ gap WRITE setGap)
    Q_PROPERTY(qint32 buttonsGap READ buttonsGap WRITE setButtonsGap)
    Q_PROPERTY(bool collapsable READ collapsable WRITE setCollapsable)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY pageTitleChanged)

public:
    explicit WidgetsTabBarLayout(QWidget *parent = nullptr);
    ~WidgetsTabBarLayout();

    const QVector<class QPushButton*>& buttons() const;

    qint32 count() const { return m_views.size(); }
    QWidget* widgetAt(qint32 index) const;
    QWidget* currentWidget() const;
    qint32 currentIndex() const;
    qint32 buttonsGap() const;
    qint32 gap() const;
    QString title() const;

    bool collapsable() const;
    void setCollapsable(bool collapsable);

    void setGap(qint32 gap);

    LocalPropertyBool Opened;

signals:
    void pageTitleChanged(const QString&);
    void currentIndexChanged(qint32);

public slots:
    void setButtonsGap(qint32 gap);
    void setTitle(const QString&);
    void addPage(QWidget *page);
    void insertPage(int index, QWidget *page);
    void removePage(int index);
    void setCurrentIndex(qint32 index);

private:
    Ui::WidgetsTabBarLayout *ui;
    QVector<QPushButton*> m_buttons;
    QVector<QWidget*> m_views;
    qint32 m_prevIndex;
    LocalPropertyInt m_currentIndex;
    class QLabel* m_icon;
    bool m_collapsable;
};

#endif // WIDGETSTABBARLAYOUT_H
