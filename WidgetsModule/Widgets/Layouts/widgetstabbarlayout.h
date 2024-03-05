#ifndef WIDGETSTABBARLAYOUT_H
#define WIDGETSTABBARLAYOUT_H

#include <QFrame>

namespace Ui {
class WidgetsTabBarLayout;
}

class WidgetsTabBarLayout : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qint32 currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(qint32 gap READ gap WRITE setGap)
    Q_PROPERTY(qint32 buttonsGap READ buttonsGap WRITE setButtonsGap)
    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    explicit WidgetsTabBarLayout(QWidget *parent = nullptr);
    ~WidgetsTabBarLayout();

    class QStackedWidget* widget() const;
    const QVector<class QPushButton*>& buttons() const;

    qint32 buttonsGap() const;
    qint32 currentIndex() const;
    qint32 gap() const;
    QString title() const;

    void setGap(qint32 gap);

public slots:
    void setButtonsGap(qint32 gap);
    void setTitle(const QString&);
    void addPage(QWidget *page);
    void insertPage(int index, QWidget *page);
    void removePage(int index);
    void setCurrentIndex(int index);

private:
    Ui::WidgetsTabBarLayout *ui;
    QVector<QPushButton*> m_buttons;
};

#endif // WIDGETSTABBARLAYOUT_H
