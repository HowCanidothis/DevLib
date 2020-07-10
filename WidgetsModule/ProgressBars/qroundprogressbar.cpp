#include "QRoundProgressBar.h"

#include <QPainter>
#include <QTimer>
#include <qmath.h>

QRoundProgressBar::QRoundProgressBar(QWidget *parent)
    : QWidget(parent)
    , m_min(0)
    , m_max(100)
    , m_value(25)
    , m_ringWidth(100)
    , m_indeterminateSpeed(40)
    , m_framer(new QTimer(this))
{    
    connect(m_framer, SIGNAL(timeout()), this, SLOT(update()));
}

void QRoundProgressBar::setRingWidth(qint32 width)
{
    if(m_ringWidth != width) {
        m_ringWidth = width;

        update();
    }
}

void QRoundProgressBar::setRange(qint32 max, qint32 min)
{
    m_min = min;
    m_max = max;

    if (m_max < m_min)
        qSwap(m_max, m_min);

    if(m_max == m_min) {
        m_framer->start(m_indeterminateSpeed);
        return;
    } else {
        m_framer->stop();
    }

    if (m_value < m_min)
        m_value = m_min;
    else if (m_value > m_max)
        m_value = m_max;

    update();
}

void QRoundProgressBar::setMinimum(qint32 min)
{
    setRange(min, m_max);
}

void QRoundProgressBar::setMaximum(qint32 max)
{
    setRange(m_min, max);
}

void QRoundProgressBar::setValue(qint32 val)
{
    if (m_value != val)
    {
        if (val < m_min)
            m_value = m_min;
        else if (val > m_max)
            m_value = m_max;
        else
            m_value = val;

        update();
    }
}

void QRoundProgressBar::setText(const QString& text)
{
    if(m_text != text)
    {
        m_text = text;

        update();
    }
}

void QRoundProgressBar::setIndeterminateSpeed(qint32 value)
{
    if(m_indeterminateSpeed != value) {
        if(m_framer->isActive()) {
            m_framer->start(value);
        }
    }
}

qint32 QRoundProgressBar::indeterminateSpeed() const
{
    return m_indeterminateSpeed;
}

void QRoundProgressBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    qint32 weight = m_ringWidth;
    QPoint center(width() / 2, height() / 2);

    qint32 outerDiameter = qMin(width(), height());
    qint32 innerDiameter = innerDiameter = outerDiameter - weight * 2;;
    qint32 sideInnerRect = innerDiameter / sqrt(2.0);

    QRect innerRect = QRect(center - QPoint(sideInnerRect / 2, sideInnerRect / 2), QSize(sideInnerRect, sideInnerRect));
    QRect innerTopRect;
    QRect innerBottomRect;
    if(!m_text.isEmpty()) {
        qint32 halfHeight = innerRect.height() / 2;
        innerTopRect = innerRect.adjusted(0,0,0,-halfHeight);
        innerBottomRect = innerRect.adjusted(0, halfHeight, 0, 0);
    } else {
        innerTopRect = innerRect;
    }
    QRect outerRect = QRect(center - QPoint(outerDiameter / 2 - weight / 2, outerDiameter / 2 - weight / 2), QSize(outerDiameter - weight, outerDiameter - weight));

    QColor baseColor = palette().base().color();
    QColor valueColor = palette().alternateBase().color();
    p.setBrush(Qt::NoBrush);

    if(m_max != m_min) {
        p.setPen(QPen(baseColor, weight, Qt::SolidLine, Qt::FlatCap));
        drawRing(p, outerRect, 360);

        float arcStep = 360.f / (m_max - m_min) * m_value - 90.f;
        p.setPen(QPen(valueColor, weight, Qt::SolidLine, Qt::FlatCap));
        drawRing(p, outerRect, arcStep);

        p.setPen(palette().foreground().color());
        p.save();
        QFont font = p.font();
        font.setPixelSize(innerTopRect.height() / 2);
        p.setFont(font);
        p.drawText(innerTopRect, Qt::AlignCenter, QString::number(qint32(float(m_value) / m_max * 100.f)) + "%");
        p.restore();
    } else {
        static qint32 angle = 0;

        if(qint32(angle / 360) % 2) {
            std::swap(baseColor, valueColor);
        }

        p.setPen(QPen(baseColor, weight, Qt::SolidLine, Qt::FlatCap));
        drawRing(p, outerRect, 360);

        p.setPen(QPen(valueColor, weight + 1.0, Qt::SolidLine, Qt::FlatCap));
        drawRing(p, outerRect, angle % 360 - 90);
        angle += 10;
    }

    // text    
    p.setPen(palette().text().color());
    p.drawText(innerBottomRect, Qt::AlignCenter|Qt::TextWordWrap, m_text);
}

void QRoundProgressBar::drawRing(QPainter& p, const QRect& rect, qint32 angleDegrees)
{
    // for Pie and Donut styles
    QPainterPath dataPath;
    dataPath.arcTo(rect, 90, -angleDegrees - 90);

    if(dataPath.elementCount() > 1) {
        auto secondElement = dataPath.elementAt(1);
        dataPath.setElementPositionAt(0, secondElement.x, secondElement.y);

        p.drawPath(dataPath);
    }
}
