#ifndef WIDGETS_MODULE_INTERNAL_HPP
#define WIDGETS_MODULE_INTERNAL_HPP

#if defined __cplusplus

#include "Actions/widgetsglobaltableactionsscope.h"
#include "Actions/actionsmanager.h"

#include "Models/viewmodeltreebase.h"
#include "Models/modelstablebase.h"
#include "Models/modelslistbase.h"
#include "Models/modelsfiltermodelbase.h"
#include "Models/viewmodelvacabulary.h"
#include "Models/viewmodelsdefaultcomponents.h"
#include "Models/viewmodelsdefaultfieldmodel.h"
#include "Models/viewmodelsstandard.h"

#include "Bars/menubarmovepane.h"
#include "Bars/widgetstoolbar.h"

#include "Notifier/notifymanager.h"
#include "Notifier/topnotifierframe.h"
#include "Notifier/notifyconsole.h"
#include "Notifier/notifybutton.h"

#include "ProgressBars/mainprogressbar.h"
#include "ProgressBars/shadowprogressbar.h"
#include "ProgressBars/shadowprogressbaroneforall.h"

#include "ToolTip/tooltipwidget.h"
#include "ToolTip/tooltiparrowwidget.h"

#include "Attachments/tableviewwidgetattachment.h"
#include "Attachments/widgetsactivetableattachment.h"
#include "Attachments/windowresizeattachment.h"
#include "Attachments/widgetslocationattachment.h"
#include "Attachments/widgetstableviewrowattachment.h"

#include "Components/componentplacer.h"

#include "Layouts/flowlayout.h"
#include "Layouts/flowwidget.h"

#include "Utils/iconsmanager.h"
#include "Utils/widgetstyleutils.h"
#include "Utils/widgetsstylesettings.h"
#include "Utils/widgetbuilder.h"
#include "Utils/widgethelpers.h"

#include "Delegates/delegates.h"

#include "TableViews/widgetstableviewbase.h"
#include "TableViews/widgetsadjustabletableview.h"
#include "TableViews/Header/widgetsresizableheaderattachment.h"

#include "Widgets/widgetsspinboxwithcustomdisplay.h"
#include "Widgets/widgetsimportview.h"
#include "Widgets/widgetseditingobjectsavecancelbuttons.h"
#include "Widgets/DateTime/widgetsdatetimeedit.h"
#include "Widgets/DateTime/widgetsdatetimepopuppicker.h"
#include "Widgets/DateTime/widgetsdatetimewidget.h"
#include "Widgets/DateTime/widgetstimepicker.h"
#include "Widgets/DateTime/widgetstimewidget.h"
#include "Widgets/DateTime/widgetscalendarwidget.h"
#include "Widgets/DateTime/widgetsmonthpicker.h"
#include "Widgets/DateTime/widgetsmonthpopuppicker.h"
#include "Widgets/elidedlabel.h"
#include "Widgets/widgetscolorpicker.h"
#include "Widgets/widgetspicturepicker.h"
#include "Widgets/propertiestoolview.h"
#include "Widgets/Layouts/internal.hpp"

//#include "Widgets/widgetsfloatingtoolbar.h"

#include "Dialogs/widgetsinputdialogview.h"
#include "Dialogs/widgetshyperlinkedlistdialogview.h"

#include "Managers/widgetsdialogsmanager.h"
#include "Managers/widgetsfocusmanager.h"
#include "Managers/modelsvocabularymanager.h"
#include "Managers/widgetsstandardtableheadermanager.h"

#include "qtqssreader.h"
#endif

#endif // INTERNAL_H
