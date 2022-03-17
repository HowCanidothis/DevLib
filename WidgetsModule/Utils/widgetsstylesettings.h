#ifndef WIDGETSSTYLESETTINGS_H
#define WIDGETSSTYLESETTINGS_H

#include <QWidget>

class WidgetsStyleSettings : public QWidget
{
    Q_OBJECT
    using Super = QWidget;

    Q_PROPERTY(QColor normalColor MEMBER m_normalColor)
    Q_PROPERTY(QColor selectionColor MEMBER m_selectionColor)
    Q_PROPERTY(QColor shadowColor MEMBER m_shadowColor)
    Q_PROPERTY(qint32 shadowBlurRadius MEMBER m_shadowBlurRadius)

    Q_PROPERTY(QColor enabledTableColor MEMBER m_enabledTableColor)
    Q_PROPERTY(QColor enabledTableTextColor MEMBER m_enabledTableTextColor)
    Q_PROPERTY(QColor disabledTableColor MEMBER m_disabledTableColor)
    Q_PROPERTY(QColor disabledTableTextColor MEMBER m_disabledTableTextColor)

public:
    WidgetsStyleSettings();

private:
    LocalPropertyColor m_enabledTableColor;
    LocalPropertyColor m_enabledTableTextColor;
    LocalPropertyColor m_disabledTableColor;
    LocalPropertyColor m_disabledTableTextColor;

    LocalPropertyColor m_normalColor;
    LocalPropertyColor m_selectionColor;
    LocalPropertyColor m_shadowColor;
    LocalPropertyInt m_shadowBlurRadius;

private:
    static WidgetsStyleSettings* m_instance;
};

#endif
