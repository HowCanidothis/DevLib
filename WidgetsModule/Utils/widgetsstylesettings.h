#ifndef WIDGETSSTYLESETTINGS_H
#define WIDGETSSTYLESETTINGS_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

class WidgetsStyleSettings : public QWidget
{
    Q_OBJECT
    using Super = QWidget;

    Q_PROPERTY(QColor normalColor MEMBER m_normalColor)
    Q_PROPERTY(QColor selectionColor MEMBER m_selectionColor)
    Q_PROPERTY(QColor secondaryColor MEMBER m_secondaryColor)
    Q_PROPERTY(QColor secondarySelectionColor MEMBER m_secondarySelectionColor)
    Q_PROPERTY(QColor disabledColor MEMBER m_disabledColor)
    Q_PROPERTY(QColor secondaryDisabledColor MEMBER m_secondaryDisabledColor)
    Q_PROPERTY(QColor shadowColor MEMBER m_shadowColor)
    Q_PROPERTY(qint32 shadowBlurRadius MEMBER m_shadowBlurRadius)

    Q_PROPERTY(QColor enabledTableColor MEMBER m_enabledTableColor)
    Q_PROPERTY(QColor enabledTableTextColor MEMBER m_enabledTableTextColor)
    Q_PROPERTY(QColor disabledTableColor MEMBER m_disabledTableColor)
    Q_PROPERTY(QColor disabledTableTextColor MEMBER m_disabledTableTextColor)
    Q_PROPERTY(QColor errorLinkColor MEMBER m_errorLinkColor)
    Q_PROPERTY(QColor warningLinkColor MEMBER m_warningLinkColor)
    Q_PROPERTY(QColor errorColor MEMBER m_errorColor)
    Q_PROPERTY(QColor warningColor MEMBER m_warningColor)

    Q_PROPERTY(qint32 showFocusMinFrame MEMBER m_showFocusMinFrame)
    Q_PROPERTY(QColor placeHolderColor MEMBER m_placeHolderColor)

public:
    WidgetsStyleSettings();

private:
    LocalPropertyColor& m_enabledTableColor;
    LocalPropertyColor& m_enabledTableTextColor;
    LocalPropertyColor& m_disabledTableColor;
    LocalPropertyColor& m_disabledTableTextColor;

    LocalPropertyColor& m_secondarySelectionColor;
    LocalPropertyColor& m_normalColor;
    LocalPropertyColor& m_secondaryColor;
    LocalPropertyColor& m_selectionColor;
    LocalPropertyColor& m_disabledColor;
    LocalPropertyColor& m_secondaryDisabledColor;
    LocalPropertyColor& m_shadowColor;
    LocalPropertyColor& m_errorLinkColor;
    LocalPropertyColor& m_errorColor;
    LocalPropertyColor& m_warningLinkColor;
    LocalPropertyColor& m_warningColor;
    LocalPropertyColor& m_placeHolderColor;
    LocalPropertyInt m_shadowBlurRadius;

    LocalPropertyInt& m_showFocusMinFrame;

private:
    static WidgetsStyleSettings* m_instance;
};

#endif
