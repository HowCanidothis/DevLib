#ifndef WIDGETSTIMEPICKER_H
#define WIDGETSTIMEPICKER_H

#include <QWidget>
#include <PropertiesModule/internal.hpp>

enum class ClockType {
    Hour,
    Minutes,
    Seconds,
    First = Hour,
    Last = Seconds,
};

template<>
struct EnumHelper<ClockType>
{
    static QStringList GetNames() { return { tr("Hour"), tr("Minutes"), tr("Seconds") }; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

class WidgetsTimePicker : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
    
    Q_PROPERTY(int labelSize MEMBER m_labelSize)
    Q_PROPERTY(int arrowWidth MEMBER m_arrowWidth)
    Q_PROPERTY(QColor arrowColor MEMBER m_arrowColor)
    Q_PROPERTY(QColor clockColor MEMBER m_clockColor)
    
public:
    WidgetsTimePicker(QWidget* parent = nullptr);

    LocalPropertyInt CurrentTime;
    LocalPropertySequentialEnum<ClockType> Type;
    
    DelayedCallDispatchersCommutator OnChanged;
    
protected:
    void drawTimeLine(QPainter* painter) const;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    QSize minimumSizeHint() const override;
    int heightForWidth(int width) const override;
    bool hasHeightForWidth() const override { return true; }

private:
    void updateCenter();
    void updateClockFace();
    void updateTimeAngle(QMouseEvent* event);

private:
    int m_count;
    QPoint m_center;
	double m_prevAngle;
	LocalPropertyDouble m_angle;
    QMap<double, class QLabel*> m_buttons;
	
    LocalPropertyInt m_labelSize;
    LocalPropertyInt m_arrowWidth;
    LocalPropertyColor m_arrowColor;
    LocalPropertyColor m_clockColor;
};

#endif // WIDGETSTIMEPICKER_H
