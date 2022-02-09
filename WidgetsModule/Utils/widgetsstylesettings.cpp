#include "widgetsstylesettings.h"

WidgetsStyleSettings* WidgetsStyleSettings::m_instance = nullptr;

WidgetsStyleSettings::WidgetsStyleSettings(QWidget* mainWidget)
    : Super(mainWidget)
{
    Q_ASSERT(m_instance == nullptr);
    m_instance = this;

    auto& settings = SharedSettings::GetInstance();
    auto& styleSettings = settings.StyleSettings;

    styleSettings.DisabledTableCellColor.ConnectFrom(m_disabledTableColor);
    styleSettings.EnabledTableCellColor.ConnectFrom(m_enabledTableColor);
    styleSettings.DisabledTableCellTextColor.ConnectFrom(m_disabledTableTextColor);
    styleSettings.EnabledTableCellTextColor.ConnectFrom(m_enabledTableTextColor);

    styleSettings.IconPrimaryColor.ConnectFrom(m_normalColor);
    styleSettings.IconSelectionColor.ConnectFrom(m_selectionColor);
    styleSettings.ShadowColor.ConnectFrom(m_shadowColor);

    auto& metricSettings = settings.MetricSettings;

    metricSettings.ShadowBlurRadius.ConnectFrom(m_shadowBlurRadius, [](qint32 radius){
        return radius;
    });
}
