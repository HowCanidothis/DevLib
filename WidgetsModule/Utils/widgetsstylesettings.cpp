#include "widgetsstylesettings.h"

WidgetsStyleSettings* WidgetsStyleSettings::m_instance = nullptr;

WidgetsStyleSettings::WidgetsStyleSettings()
    : m_enabledTableColor(SharedSettings::GetInstance().StyleSettings.EnabledTableCellColor)
    , m_enabledTableTextColor(SharedSettings::GetInstance().StyleSettings.EnabledTableCellTextColor)
    , m_disabledTableColor(SharedSettings::GetInstance().StyleSettings.DisabledTableCellColor)
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
    , m_showFocusMinFrame(SharedSettings::GetInstance().StyleSettings.ShowFocusMinFrame)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;

    auto& settings = SharedSettings::GetInstance();

    auto& metricSettings = settings.MetricSettings;

    metricSettings.ShadowBlurRadius.ConnectFrom(CONNECTION_DEBUG_LOCATION, [](qint32 radius){
        return radius;
    }, m_shadowBlurRadius);
}
