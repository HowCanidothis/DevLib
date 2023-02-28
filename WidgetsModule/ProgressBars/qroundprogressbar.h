#ifndef QROUNDPROGRESSBAR_H
#define QROUNDPROGRESSBAR_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

class QRoundProgressBar : public QWidget
{
    Q_OBJECT
    using Super = QWidget;
    Q_PROPERTY(qint32 ringWidth MEMBER RingWidth)
    Q_PROPERTY(qint32 ringSize MEMBER RingSize)
public:
    explicit QRoundProgressBar(QWidget *parent = 0);

    LocalPropertyInt RingSize;
    LocalPropertyInt RingWidth;
    LocalProperty<QSize> Range;
    LocalPropertyString Text;
    LocalPropertyInt Value;

    void SetMaximum(qint32 maximum);

protected:
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void drawRing(QPainter& p, const QRect& rect, qint32 angleDegrees);
    bool isIndeterminate() const { return Range.Native().width() == Range.Native().height(); }

    virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE { return QSize(32,32); }

    virtual bool hasHeightForWidth() const Q_DECL_OVERRIDE { return true; }
    virtual int heightForWidth(int w) const Q_DECL_OVERRIDE { return w; }

    DispatchersCommutator m_update;
    QtLambdaConnections m_qtconnections;
};

#endif // QROUNDPROGRESSBAR_H
