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

enum class HourFormat {
    Hour12,
    Hour24,
    First = Hour12,
    Last = Hour24,
};

template<>
struct EnumHelper<ClockType>
{
    static QStringList GetNames() { return { tr("Hour"), tr("Minutes"), tr("Seconds") }; }
    Q_DECLARE_TR_FUNCTIONS(EnumHelper)
};

class QLabel;
class WidgetsTimePicker : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
    
    Q_PROPERTY(int labelSize MEMBER m_labelSize)
    Q_PROPERTY(int arrowWidth MEMBER m_arrowWidth)
    Q_PROPERTY(QColor arrowColor MEMBER m_arrowColor)
    Q_PROPERTY(QColor clockColor MEMBER m_clockColor)
    
    struct ClockLabel
    {
        ClockLabel(QWidget* parent);

        QLabel* In;
        QLabel* Out;
    };

public:
    WidgetsTimePicker(QWidget* parent = nullptr);

    LocalPropertyInt CurrentTime;
    LocalPropertySequentialEnum<HourFormat> HourType;
    LocalPropertySequentialEnum<ClockType> TypeClock;
    
    DispatchersCommutator OnChanged;
    
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
    inline int sectionsCount() const { return TypeClock.Native() == ClockType::Hour ? 12 : 60; }

private:
    LocalPropertyBool m_isOut;
    LocalPropertyDouble m_angle;
    LocalProperty<QPoint> m_centerPos;
    LocalProperty<QPoint> m_cursorPos;
    LocalPropertyPtr<QLabel> m_activeLabel;
    DispatchersCommutator m_timeChanged;
    DispatcherConnectionsSafe m_connections;

    QMap<double, ClockLabel> m_labels;

    LocalPropertyInt m_labelSize;
    LocalPropertyInt m_arrowWidth;
    LocalPropertyColor m_arrowColor;
    LocalPropertyColor m_clockColor;
};

#endif // WIDGETSTIMEPICKER_H
