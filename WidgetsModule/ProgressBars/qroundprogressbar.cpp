#include "QRoundProgressBar.h"

#include <QPainter>
#include <QTimer>
#include <qmath.h>

#include "WidgetsModule/Utils/widgethelpers.h"

class GlobalTimer : public Singletone<GlobalTimer>
{
public:
    GlobalTimer()
        : Value(0)
    {
        QObject::connect(&Timer, &QTimer::timeout, [this]{
            Value += 10;
        });
        Timer.start(10);
    }

    QTimer Timer;
    qint32 Value;
};

QRoundProgressBar::QRoundProgressBar(QWidget *parent)
    : QWidget(parent)
    , RingSize(100)
    , RingWidth(10)
    , Range(QSize(0, 100))
{
    if(!GlobalTimer::IsInitialized()) {
        new GlobalTimer();
    }

    m_qtconnections.connect(&GlobalTimer::GetInstance().Timer, &QTimer::timeout, [this]{
        if(isIndeterminate() && isVisible()) {
            Value = GlobalTimer::GetInstance().Value;
        }
    });

    m_update.Connect(CONNECTION_DEBUG_LOCATION, [this]{
        if(isVisible()) {
            update();
        }
    });

    Range.SetValidator([](const QSize& value){
        QSize copySize = value;
        if(copySize.height() < copySize.width()) {
            std::swap(copySize.rwidth(), copySize.rheight());
        }
        return copySize;
    });
    Value.SetValidator([this](qint32 value){
        if(isIndeterminate()) {
            return value;
        }
        return clamp(value, Range.Native().width(), Range.Native().height());
    });
    Value.ConnectFromDispatchers(CONNECTION_DEBUG_LOCATION, [this]{
        return Value.Native();
    }, Range);

    Text.Connect(CONNECTION_DEBUG_LOCATION, [this](const QString& text) {
        setToolTip(text);
    });

    m_update.ConnectFrom(CONNECTION_DEBUG_LOCATION, Range, Value, RingWidth, RingSize);
}

void QRoundProgressBar::SetMaximum(qint32 maximum)
{
    Range = QSize(Range.Native().width(), maximum);
}

void QRoundProgressBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    qint32 weight = RingWidth;
    QPoint center(width() / 2, height() / 2);

    qint32 outerDiameter = RingSize;
    qint32 innerDiameter = outerDiameter - weight * 2;;
    qint32 sideInnerRect = innerDiameter / sqrt(2.0);

    QRect innerRect = QRect(center - QPoint(sideInnerRect / 2, sideInnerRect / 2), QSize(sideInnerRect, sideInnerRect));
    QRect innerTopRect;
    innerTopRect = innerRect;
    QRect outerRect = QRect(center - QPoint(outerDiameter / 2 - weight / 2, outerDiameter / 2 - weight / 2), QSize(outerDiameter - weight, outerDiameter - weight));

    QColor baseColor = palette().base().color();
    QColor valueColor = palette().alternateBase().color();
    p.setBrush(palette().background().color());
    p.drawRect(QRect(0,0,width(), height()));

    p.setBrush(Qt::NoBrush);

    auto min = Range.Native().width(), max = Range.Native().height();

    if(max != min) {
        p.setPen(QPen(baseColor, weight, Qt::SolidLine, Qt::FlatCap));
        drawRing(p, outerRect, 360);

        float arcStep = 360.f / (max - min) * Value - 90.f;
        p.setPen(QPen(valueColor, weight, Qt::SolidLine, Qt::FlatCap));
        drawRing(p, outerRect, arcStep);

        p.setPen(palette().foreground().color());
        p.save();
        p.drawText(innerTopRect, Qt::AlignCenter, QString::number(qint32(float(Value) / max * 100.f)) + "%");
        p.restore();
    } else {
        if(qint32(Value / 360) % 2) {
            std::swap(baseColor, valueColor);
        }

        p.setPen(QPen(baseColor, weight, Qt::SolidLine, Qt::FlatCap));
        drawRing(p, outerRect, 360);

        p.setPen(QPen(valueColor, weight + 1.0, Qt::SolidLine, Qt::FlatCap));
        drawRing(p, outerRect, Value % 360 - 90);
    }
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
