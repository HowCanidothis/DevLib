#include "widgetstimepicker.h"
#include <qmath.h>

#include <QLabel>
#include <QMouseEvent>
#include "WidgetsModule/Utils/styleutils.h"

WidgetsTimePicker::WidgetsTimePicker(QWidget* parent)
    : Super(parent)
    , CurrentTime(0)
    , TypeClock(ClockType::First)
    , m_activeLabel(nullptr)
    , m_isOut(true)
    , m_angle(-1)
{
    for(int i=0; i<12; ++i){
        auto iter = m_labels.insert(round<8>(2.*M_PI*(double)i/12.), {this});
        iter.value().In->setText(QString::number(i+12));
    }

    m_angle.OnChanged.ConnectAndCallCombined(CONNECTION_DEBUG_LOCATION, [this]{
        if(m_activeLabel) {
            m_activeLabel->setProperty("active", false);
            StyleUtils::UpdateStyle(m_activeLabel);
        }
        auto iter = m_labels.find(m_angle);
        if(iter != m_labels.end()){
            m_activeLabel = m_isOut ? iter.value().Out : iter.value().In;
            m_activeLabel->setProperty("active", true);
            StyleUtils::UpdateStyle(m_activeLabel);
        } else {
            m_activeLabel = nullptr;
        }
    }, m_isOut.OnChanged);

    TypeClock.OnChanged.ConnectAndCallCombined(CONNECTION_DEBUG_LOCATION, [this]{
        switch(TypeClock.Native()){
        case ClockType::Hour: {
            int i=0;
            for(auto& lb : m_labels){
                lb.Out->setText(QString::number(i++));
                lb.In->setVisible(HourType.Native() == HourFormat::Hour24);
            }
            break;
        }
        case ClockType::Minutes:
        case ClockType::Seconds: {
            int i=0;
            for(auto& lb : m_labels){
                lb.Out->setText(QString::number(i));
                lb.In->setVisible(false);
                i += 5;
            }
            break;
        }}

        m_connections.clear();
        static auto timeConverter ([](const int& time, const int& count){ return round<8>(2 * M_PI * time / count); });
        if(HourType.Native() == HourFormat::Hour24){
            LocalPropertiesConnectBoth(CONNECTION_DEBUG_LOCATION, {&CurrentTime.OnChanged}, [this]{
                m_isOut = CurrentTime < sectionsCount();
                m_angle = timeConverter(CurrentTime.Native() - (m_isOut ? 0 : 12), sectionsCount());
            }, {&m_angle.OnChanged, &m_isOut.OnChanged}, [this]{
                CurrentTime = round(m_angle * sectionsCount() / ( M_PI*2)) + (m_isOut ? 0 : 12);
            }).MakeSafe(m_connections);
        } else {
            CurrentTime.ConnectBoth(CONNECTION_DEBUG_LOCATION,m_angle, [this](int time){
                return timeConverter(time, sectionsCount());
            }, [this](double angle){
                return round(angle * sectionsCount() / ( M_PI*2));
            }).MakeSafe(m_connections);
        }
    }, HourType.OnChanged);

    m_cursorPos.Subscribe([this]{
        auto delta = m_centerPos.Native() - m_cursorPos.Native();
        auto degree = -qRadiansToDegrees(atan2(delta.x(), delta.y()));
        auto angle = qDegreesToRadians( 360.0/sectionsCount() * qRound(degree/(360.0/sectionsCount())));

        if(angle < 0) angle += 2.*M_PI;
        m_angle = round<8>(angle);

        if(TypeClock.Native() == ClockType::Hour && HourType.Native() == HourFormat::Hour24){
            auto length = sqrt(pow(delta.x(), 2) + pow(delta.y(), 2));
            m_isOut = length > m_centerPos.Native().x() - m_labelSize;
        } else {
            m_isOut = true;
        }
    });
    m_centerPos.SetAndSubscribe([this]{
        auto radiusOut = m_centerPos.Native().x() - m_labelSize/2;
        auto radiusIn = radiusOut - m_labelSize;

        static auto position = [](const double& angle, const int& radius) -> QPoint { return QPoint(radius + round(sin(angle) * radius), radius - round(cos(angle) * radius)); };
        for(auto iter = m_labels.begin(); iter != m_labels.end(); ++iter){
            const auto& angle = iter.key();
            auto positionIn = position(angle, radiusIn);
            iter.value().In->setGeometry(positionIn.x() + m_labelSize, positionIn.y() + m_labelSize, m_labelSize, m_labelSize);

            auto positionOut = position(angle, radiusOut);
            iter.value().Out->setGeometry(positionOut.x(), positionOut.y(), m_labelSize, m_labelSize);
        }
    });

    OnChanged.ConnectFrom(CONNECTION_DEBUG_LOCATION,
        m_labelSize.OnChanged,
        m_arrowWidth.OnChanged,
        m_arrowColor.OnChanged,
        m_clockColor.OnChanged,
        CurrentTime.OnChanged,
        TypeClock.OnChanged
    );
    OnChanged.Connect(CONNECTION_DEBUG_LOCATION, [this]{update();});
}

void WidgetsTimePicker::drawTimeLine(QPainter* painter) const
{
    painter->setBrush(QBrush(m_clockColor));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(m_centerPos, m_centerPos.Native().x(), m_centerPos.Native().y());

    auto length = m_labelSize/2 + (m_isOut.Native() ? 0 : m_labelSize.Native());
    auto deltaX = (m_centerPos.Native().x() - length) * sin(m_angle);
    auto deltaY = (length - m_centerPos.Native().y())*cos(m_angle);

    painter->setBrush(QBrush(QColor(m_arrowColor)));
    painter->drawEllipse(m_centerPos.Native(), m_arrowWidth, m_arrowWidth);
    painter->setPen(QPen(QColor(m_arrowColor), m_arrowWidth));
    painter->drawLine(m_centerPos, QPoint(deltaX, deltaY) + m_centerPos.Native());
}

void WidgetsTimePicker::paintEvent(QPaintEvent* event)
{
    Super::paintEvent(event);
    QPainter p(this); p.setRenderHints( QPainter::HighQualityAntialiasing );
    drawTimeLine(&p);
}

void WidgetsTimePicker::resizeEvent(QResizeEvent* event)
{
    auto side = qMin(geometry().width(), geometry().height())/2;
    m_centerPos = {side, side};
    Super::resizeEvent(event);
}

void WidgetsTimePicker::mouseMoveEvent(QMouseEvent* event)
{
    m_cursorPos = event->pos();
    Super::mouseMoveEvent(event);
}

void WidgetsTimePicker::mousePressEvent(QMouseEvent* event)
{
    m_cursorPos = event->pos();
    Super::mousePressEvent(event);
}

void WidgetsTimePicker::mouseReleaseEvent(QMouseEvent* event)
{
    if(QVector2D(event->pos() - m_centerPos).length() < m_centerPos.Native().x()) {
        OnMouseReleased();
    }
}

QSize WidgetsTimePicker::minimumSizeHint() const
{
    return {180, 180};
}

int WidgetsTimePicker::heightForWidth(int width) const
{
    return width;
}

WidgetsTimePicker::ClockLabel::ClockLabel(QWidget* parent)
    : In(new QLabel(parent))
    , Out(new QLabel(parent))
{
    In->setAlignment(Qt::AlignCenter);
    Out->setAlignment(Qt::AlignCenter);

    In->setProperty("active", false);
    Out->setProperty("active", false);
}
