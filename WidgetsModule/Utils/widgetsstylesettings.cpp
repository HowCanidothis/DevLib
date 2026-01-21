#include "widgetsstylesettings.h"

#ifdef VISUALIZATION_MODULE_LIB
#include <VisualizationModule/internal.hpp>
#endif

WidgetsStyleSettings* WidgetsStyleSettings::m_instance = nullptr;

WidgetsStyleSettings::WidgetsStyleSettings()
    : m_enabledTableColor(SharedSettings::GetInstance().StyleSettings.EnabledTableCellColor)
    , m_enabledTableAltColor(SharedSettings::GetInstance().StyleSettings.EnabledTableCellAltColor)
    , m_enabledTableTextColor(SharedSettings::GetInstance().StyleSettings.EnabledTableCellTextColor)
    , m_disabledTableColor(SharedSettings::GetInstance().StyleSettings.DisabledTableCellColor)
    , m_disabledTableAltColor(SharedSettings::GetInstance().StyleSettings.DisabledTableCellAltColor)
    , m_disabledTableTextColor(SharedSettings::GetInstance().StyleSettings.DisabledTableCellTextColor)
    , m_secondarySelectionColor(SharedSettings::GetInstance().StyleSettings.IconSecondarySelectionColor)
    , m_normalColor(SharedSettings::GetInstance().StyleSettings.IconPrimaryColor)
    , m_secondaryColor(SharedSettings::GetInstance().StyleSettings.IconSecondaryColor)
    , m_selectionColor(SharedSettings::GetInstance().StyleSettings.IconSelectionColor)
    , m_disabledColor(SharedSettings::GetInstance().StyleSettings.IconDisabledColor)
    , m_secondaryDisabledColor(SharedSettings::GetInstance().StyleSettings.IconSecondaryDisabledColor)
    , m_shadowColor(SharedSettings::GetInstance().StyleSettings.ShadowColor)
    , m_errorLinkColor(SharedSettings::GetInstance().StyleSettings.ErrorLinkColor)
    , m_errorColor(SharedSettings::GetInstance().StyleSettings.ErrorColor)
    , m_warningLinkColor(SharedSettings::GetInstance().StyleSettings.WarningLinkColor)
    , m_warningColor(SharedSettings::GetInstance().StyleSettings.WarningColor)
    , m_placeHolderColor(SharedSettings::GetInstance().StyleSettings.PlaceHolderColor)
    , m_defaultBlueColor(SharedSettings::GetInstance().StyleSettings.DefaultBlueColor)
    , m_errorCellColor(SharedSettings::GetInstance().StyleSettings.ErrorCellColor)
    , m_errorCellAltColor(SharedSettings::GetInstance().StyleSettings.ErrorCellAltColor)
    , m_warningCellColor(SharedSettings::GetInstance().StyleSettings.WarningCellColor)
    , m_warningCellAltColor(SharedSettings::GetInstance().StyleSettings.WarningCellAltColor)
    , m_showFocusMinFrame(SharedSettings::GetInstance().StyleSettings.ShowFocusMinFrame)
#ifdef VISUALIZATION_MODULE_LIB
    , m_visualizationBackgroundColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->SpaceColor)
    , m_visualizationBoxColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->BackgroundColor)
    , m_visualizationAxisColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->AxisColor)
    , m_visualizationTextColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->TextSettings.Color)
    , m_visualizationTextBorderColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->TextSettings.BorderColor)
    , m_visualizationFrameColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->FrameColor)
    , m_visualizationAltFrameColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->AltFrameColor)
    , m_visualizationAltTextColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->AltTextColor)
    , m_visualizationAltBackgroundColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->AltSpaceColor)
    , m_visualizationAltAxisColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->AltAxisColor)
    , m_visualizationFrameSelectionColor(SharedSettings::GetInstance().StyleSettings.VisualSettings->FrameSelectionColor)
    , m_visualizationAreaAlpha(SharedSettings::GetInstance().StyleSettings.VisualSettings->AreaAlpha)
#endif
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;

    auto& settings = SharedSettings::GetInstance();

    auto& metricSettings = settings.MetricSettings;

    metricSettings.ShadowBlurRadius.ConnectFrom(CONNECTION_DEBUG_LOCATION, [](qint32 radius){
        return radius;
    }, m_shadowBlurRadius);
}
