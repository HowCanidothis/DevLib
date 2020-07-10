/*
 * QRoundProgressBar - a circular progress bar Qt widget.
 *
 * Sintegrial Technologies (c) 2015-now
 *
 * The software is freeware and is distributed "as is" with the complete source codes.
 * Anybody is free to use it in any software projects, either commercial or non-commercial.
 * Please do not remove this copyright message and remain the name of the author unchanged.
 * It is very appreciated if you produce some feedback to us case you are going to use
 * the software.
 *
 * Please send yours questions, suggestions, and information about found issues to the
 *
 * sintegrial@gmail.com
 *
 */

#ifndef QROUNDPROGRESSBAR_H
#define QROUNDPROGRESSBAR_H

#include <QWidget>

/**
 * @brief The QRoundProgressBar class represents a circular progress bar and maintains its API
 *
 * via QSS:
 * background: default ring color;
 * alternate-background: processed ring color;
 * color: text color
 * qproperty-ringWidth: width of ring
 */
class QRoundProgressBar : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(qint32 ringWidth READ ringWidth WRITE setRingWidth)
public:
    explicit QRoundProgressBar(QWidget *parent = 0);

    void setRingWidth(qint32 width);
    qint32 ringWidth() const { return m_ringWidth; }

    void setText(const QString& text);
    const QString& text() const { return m_text; }

    qint32 value() const { return m_value; }
    qint32 minimum() const { return m_min; }
    qint32 maximum() const { return m_max; }

    void setIndeterminateSpeed(qint32 value);
    qint32 indeterminateSpeed() const;

public Q_SLOTS:
    void setRange(qint32 max, qint32 min);
    void setMinimum(qint32 min);
    void setMaximum(qint32 max);

    void setValue(qint32 val);

protected:
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void drawRing(QPainter& p, const QRect& rect, qint32 angleDegrees);

    virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE { return QSize(32,32); }

    virtual bool hasHeightForWidth() const Q_DECL_OVERRIDE { return true; }
    virtual int heightForWidth(int w) const Q_DECL_OVERRIDE { return w; }

    qint32 m_min, m_max;
    qint32 m_value;
    qint32 m_ringWidth;

    QString m_text;
    qint32 m_indeterminateSpeed;
    class QTimer* m_framer;
};

#endif // QROUNDPROGRESSBAR_H
