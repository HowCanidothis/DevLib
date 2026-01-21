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
    Q_PROPERTY(QColor enabledTableAltColor MEMBER m_enabledTableAltColor)
    Q_PROPERTY(QColor enabledTableTextColor MEMBER m_enabledTableTextColor)
    Q_PROPERTY(QColor disabledTableColor MEMBER m_disabledTableColor)
    Q_PROPERTY(QColor disabledTableAltColor MEMBER m_disabledTableAltColor)
    Q_PROPERTY(QColor disabledTableTextColor MEMBER m_disabledTableTextColor)
    Q_PROPERTY(QColor errorLinkColor MEMBER m_errorLinkColor)
    Q_PROPERTY(QColor warningLinkColor MEMBER m_warningLinkColor)
    Q_PROPERTY(QColor errorColor MEMBER m_errorColor)
    Q_PROPERTY(QColor warningColor MEMBER m_warningColor)

    Q_PROPERTY(QColor defaultBlueColor MEMBER m_defaultBlueColor)
    Q_PROPERTY(QColor errorCellColor MEMBER m_errorCellColor)
    Q_PROPERTY(QColor errorCellAltColor MEMBER m_errorCellAltColor)
    Q_PROPERTY(QColor warningCellColor MEMBER m_warningCellColor)
    Q_PROPERTY(QColor warningCellAltColor MEMBER m_warningCellAltColor)

    Q_PROPERTY(qint32 showFocusMinFrame MEMBER m_showFocusMinFrame)
    Q_PROPERTY(QColor placeHolderColor MEMBER m_placeHolderColor)

#ifdef VISUALIZATION_MODULE_LIB
    Q_PROPERTY(QColor visualizationBackgroundColor MEMBER m_visualizationBackgroundColor)
    Q_PROPERTY(QColor visualizationBoxColor MEMBER m_visualizationBoxColor)
    Q_PROPERTY(QColor visualizationAxisColor MEMBER m_visualizationAxisColor)
    Q_PROPERTY(QColor visualizationTextColor MEMBER m_visualizationTextColor)
    Q_PROPERTY(QColor visualizationTextBorderColor MEMBER m_visualizationTextBorderColor)
    Q_PROPERTY(QColor visualizationFrameColor MEMBER m_visualizationFrameColor)
    Q_PROPERTY(QColor visualizationAltFrameColor MEMBER m_visualizationAltFrameColor)
    Q_PROPERTY(QColor visualizationAltTextColor MEMBER m_visualizationAltTextColor)
    Q_PROPERTY(QColor visualizationAltBackgroundColor MEMBER m_visualizationAltBackgroundColor)
    Q_PROPERTY(QColor visualizationAltAxisColor MEMBER m_visualizationAltAxisColor)
    Q_PROPERTY(QColor visualizationFrameSelectionColor MEMBER m_visualizationFrameSelectionColor)
    Q_PROPERTY(float visualizationAreaAlpha MEMBER m_visualizationAreaAlpha)
#endif
public:
    WidgetsStyleSettings();

private:
    LocalPropertyColor& m_enabledTableColor;
    LocalPropertyColor& m_enabledTableAltColor;
    LocalPropertyColor& m_enabledTableTextColor;
    LocalPropertyColor& m_disabledTableColor;
    LocalPropertyColor& m_disabledTableAltColor;
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

    LocalPropertyColor& m_defaultBlueColor;
    LocalPropertyColor& m_errorCellColor;
    LocalPropertyColor& m_errorCellAltColor;
    LocalPropertyColor& m_warningCellColor;
    LocalPropertyColor& m_warningCellAltColor;

    LocalPropertyInt& m_showFocusMinFrame;
#ifdef VISUALIZATION_MODULE_LIB
    LocalPropertyColor& m_visualizationBackgroundColor;
    LocalPropertyColor& m_visualizationBoxColor;
    LocalPropertyColor& m_visualizationAxisColor;
    LocalPropertyColor& m_visualizationTextColor;
    LocalPropertyColor& m_visualizationTextBorderColor;
    LocalPropertyColor& m_visualizationFrameColor;
    LocalPropertyColor& m_visualizationAltFrameColor;
    LocalPropertyColor& m_visualizationAltTextColor;
    LocalPropertyColor& m_visualizationAltBackgroundColor;
    LocalPropertyColor& m_visualizationAltAxisColor;
    LocalPropertyColor& m_visualizationFrameSelectionColor;
    LocalPropertyFloat& m_visualizationAreaAlpha;
#endif
private:
    static WidgetsStyleSettings* m_instance;
};

#endif
