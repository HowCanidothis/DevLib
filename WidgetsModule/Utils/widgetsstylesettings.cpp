#include "widgetsstylesettings.h"

WidgetsStyleSettings* WidgetsStyleSettings::m_instance = nullptr;

WidgetsStyleSettings::WidgetsStyleSettings()
    : m_enabledTableColor(Qt::white)
    , m_disabledTableColor("#d4d4d4")
    , m_showFocusMinFrame(3)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;

    auto& settings = SharedSettings::GetInstance();
    auto& styleSettings = settings.StyleSettings;

    styleSettings.DisabledTableCellColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_disabledTableColor);
    styleSettings.EnabledTableCellColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_enabledTableColor);
    styleSettings.DisabledTableCellTextColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_disabledTableTextColor);
    styleSettings.EnabledTableCellTextColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_enabledTableTextColor);

    styleSettings.IconPrimaryColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_normalColor);
    styleSettings.IconSecondaryColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_secondaryColor);
    styleSettings.IconSelectionColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_selectionColor);
    styleSettings.IconSecondarySelectionColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_secondarySelectionColor);
    styleSettings.IconDisabledColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_disabledColor);
    styleSettings.IconSecondaryDisabledColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_secondaryDisabledColor);
    styleSettings.ShadowColor.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_shadowColor);

    styleSettings.ShowFocusMinFrame.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_showFocusMinFrame);

    auto& metricSettings = settings.MetricSettings;

    metricSettings.ShadowBlurRadius.ConnectFrom(CONNECTION_DEBUG_LOCATION, m_shadowBlurRadius, [](qint32 radius){
        return radius;
    });
}
