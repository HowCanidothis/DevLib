#include "componentplacer.h"

#include <QWidget>
#include <QResizeEvent>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Utils/styleutils.h"

ComponentPlacer::ComponentPlacer(const DelayedCallObjectParams& params)
    : m_locator([]{ return QPoint(); })
    , m_locate(params)
{
}

void ComponentPlacer::Initialize()
{
    auto fupdateLocation = [this]{
        updateLocation();        
    };

    ParentSize.Subscribe(fupdateLocation);
    TargetSize.Subscribe(fupdateLocation);
    Offset.Subscribe(fupdateLocation);

    Location.Subscribe([this]{
        updateLocator();
    });
    updateLocator();
}

void ComponentPlacer::updateLocator()
{
    switch (Location.Native()) {
    case QuadTreeF::Location_TopLeft: m_locator = [this] {
            ResultPosition = Offset.Native();
        }; break;
    case QuadTreeF::Location_TopRight: m_locator = [this]{
            ResultPosition = QPoint(width() - offsetX() - targetWidth(),offsetY());
        }; break;
    case QuadTreeF::Location_BottomLeft: m_locator = [this]{
            ResultPosition = QPoint(offsetX(), height() - offsetY() - targetHeight());
        }; break;
    case QuadTreeF::Location_BottomRight: m_locator = [this]{
            ResultPosition = QPoint(width() - offsetX() - targetWidth(), height() - offsetY() - targetHeight());
        }; break;
    case QuadTreeF::Location_MiddleTop: m_locator = [this]{
            ResultPosition = QPoint(width() / 2 + offsetX() - targetWidth() / 2, offsetY());
        }; break;
    case QuadTreeF::Location_MiddleLeft: m_locator = [this]{
            ResultPosition = QPoint(offsetX(), height() / 2 - targetHeight() / 2 + offsetY());
        }; break;
    case QuadTreeF::Location_MiddleRight: m_locator = [this]{
            ResultPosition = QPoint(width() - targetWidth() - offsetX(), height() / 2 - targetHeight() / 2 + offsetY());
        }; break;
    case QuadTreeF::Location_MiddleBottom: m_locator = [this]{
            ResultPosition = QPoint(width() / 2 + offsetX() - targetWidth() / 2, height() - targetHeight() - offsetY());
        }; break;
    case QuadTreeF::Location_Center: m_locator = [this]{
            ResultPosition = QPoint((width() - targetWidth()) / 2, (height() - targetHeight()) / 2);
        }; break;
    default: m_locator = [this]{
            ResultPosition = QPoint(width() / 2 + offsetX(), height() / 2 + offsetY());
        }; break;
    }

    updateLocation();
}

void ComponentPlacer::updateLocation()
{
    m_locate.Call(CONNECTION_DEBUG_LOCATION, m_locator);
}
