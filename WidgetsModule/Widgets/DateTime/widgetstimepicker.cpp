#include "widgetstimepicker.h"
#include <qmath.h>

#include <QLabel>
#include <QMouseEvent>

WidgetsTimePicker::WidgetsTimePicker(QWidget* parent)
    : Super(parent)
    , CurrentTime(0)
	, Type(ClockType::First)
	, m_prevAngle(0.0)
{
    for(int i=0; i<12; ++i){
        auto* lb = new QLabel(this);
		lb->setAlignment(Qt::AlignCenter);
        lb->setEnabled(false);
        
        auto angleRad = round<8>(2*M_PI*i/12);
        m_buttons.insert(angleRad, lb);
    }
	Type.SetAndSubscribe([this]{
		int i=0;
		switch(Type.Native()){
		case ClockType::Hour: {
			m_count = 12; 
			for(auto* btn : m_buttons){
				btn->setText(QString::number(i ? i : 12));
				++i;
			}
			break;
		}
		case ClockType::Minutes:
		case ClockType::Seconds: {
			m_count = 60;
			for(auto* btn : m_buttons){
				btn->setText(QString::number(i));
				i += 5;
			}
			break;
		}}
		
	});
	
    static std::function<double(const int&, const int&)> timeConverter ([](const int& time, const int& count){return round<8>(2 * M_PI * time / count);});
	CurrentTime.ConnectBoth(m_angle, [this](const int& time){
        return timeConverter(time, m_count);
	}, [this](const double& angle){
		return round(angle * m_count / ( M_PI*2));
	});
    auto updateAngleMinMax = [this]{
        m_angle.SetMinMax(timeConverter(CurrentTime.GetMin(), m_count), timeConverter(CurrentTime.GetMax(), m_count));
    };
    CurrentTime.OnMinMaxChanged.Connect(this, updateAngleMinMax);
    updateAngleMinMax();

	m_angle.SetAndSubscribe([this]{
		auto iter = m_buttons.find(m_prevAngle);
		if(iter != m_buttons.end()){
			iter.value()->setEnabled(false);
		}
		m_prevAngle = m_angle;
		iter = m_buttons.find(m_angle);
		if(iter != m_buttons.end()){
			iter.value()->setEnabled(true);
		}
	});
	
	OnChanged.Subscribe({
	    &m_labelSize.OnChange,
	    &m_arrowWidth.OnChange,
	    &m_arrowColor.OnChange,
	    &m_clockColor.OnChange,
	    &CurrentTime.OnChange,
	    &Type.OnChange,
	});
	OnChanged.Connect(this, [this]{update();});
	
    updateCenter();
    updateClockFace();
}

void WidgetsTimePicker::drawTimeLine(QPainter* painter) const
{
    auto deltaX = (m_center.x() - m_labelSize/2)*sin(m_angle);
    auto deltaY = (m_labelSize/2 - m_center.y())*cos(m_angle);
    painter->setBrush(QBrush(m_clockColor));
    painter->setPen(Qt::NoPen);
    auto d = qMin(m_center.x(), m_center.y());
    painter->drawEllipse(m_center, d, d);

    painter->setBrush(QBrush(QColor(m_arrowColor)));
    painter->drawEllipse(m_center, m_arrowWidth, m_arrowWidth);
    painter->setPen(QPen(QColor(m_arrowColor), m_arrowWidth));
    painter->drawLine(m_center, QPointF(deltaX, deltaY) + m_center);
}

void WidgetsTimePicker::paintEvent(QPaintEvent* event)
{
    Super::paintEvent(event);
    QPainter p(this); p.setRenderHints( QPainter::HighQualityAntialiasing );
    drawTimeLine(&p);
}

void WidgetsTimePicker::resizeEvent(QResizeEvent* event)
{
    updateCenter();
    updateClockFace();
    Super::resizeEvent(event);
}

void WidgetsTimePicker::mouseMoveEvent(QMouseEvent* event)
{
    updateTimeAngle(event);
    Super::mouseMoveEvent(event);
}

void WidgetsTimePicker::mousePressEvent(QMouseEvent* event)
{
    updateTimeAngle(event);
    Super::mousePressEvent(event);
}

void WidgetsTimePicker::updateCenter()
{
    auto side = qMin(geometry().width(), geometry().height())/2;
    m_center = {side, side};
}

void WidgetsTimePicker::updateClockFace() {
    auto radius = m_center.x() - m_labelSize/2;
    for(auto iter = m_buttons.begin(); iter != m_buttons.end(); ++iter){
        auto x = sin(iter.key()) * radius;
        auto y = cos(iter.key()) * radius;
        x = radius + round(x);
        y = radius - round(y);
        iter.value()->setGeometry(x,y, m_labelSize, m_labelSize);
    }
}

void WidgetsTimePicker::updateTimeAngle(QMouseEvent* event)
{
    auto delta = m_center - event->pos();
    auto degree = -qRadiansToDegrees(atan2(delta.x(), delta.y()));
	auto angle = qDegreesToRadians( 360.0/m_count * qRound(degree/(360.0/m_count)));
	if(angle < 0) angle += 2*M_PI;
    m_angle = round<8>(angle);
}

QSize WidgetsTimePicker::minimumSizeHint() const
{
    return {150, 150};
}

int WidgetsTimePicker::heightForWidth(int width) const
{
    return width;
}
