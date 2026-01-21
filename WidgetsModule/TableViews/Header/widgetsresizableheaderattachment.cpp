#include "widgetsresizableheaderattachment.h"

#include <QTableView>
#include <QWidgetAction>
#include <QMenu>
#include <QPushButton>
#include <QMouseEvent>
#include <QApplication>

#include <SharedModule/hacks.h>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Actions/widgetsglobaltableactionsscope.h"
#include "widgetsresizableheaderattachmentshowcolumnswidget.h"

class WidgetsResizableHeaderAttachmentShowColumnsButton : public QPushButton
{
    Q_OBJECT
    using Super = QPushButton;
public:
    WidgetsResizableHeaderAttachmentShowColumnsButton(QWidget* parent = nullptr)
        : Super(parent)
        , m_current(nullptr)
        , m_currentSection(-1)
        , m_blockHide(false)
    {
        setObjectName("ColumnsSelector");
        WidgetAbstractButtonWrapper(this).SetControl(ButtonRole::Icon).SetIcon(ActionIcons::Burger);
        auto* menu = new QMenu(this);
        auto* actionWidget = new QWidgetAction(menu);
        actionWidget->setDefaultWidget(m_widget = new WidgetsResizableHeaderAttachmentShowColumnsWidget());
        menu->addAction(actionWidget);
        setMenu(menu);

        connect(qApp, &QApplication::aboutToQuit, [this] {
            delete this;
        });
        connect(menu, &QMenu::aboutToShow, [this]{
            m_blockHide = true;
        });
        connect(menu, &QMenu::aboutToHide, [this]{
            m_blockHide = false;
            Hide();
        });
    }

    void Show(QHeaderView* tv, qint32 logicalIndex, const DescTableViewParams& params)
    {
        if(m_current == tv && logicalIndex == m_currentSection) {
            return;
        }
        m_current = tv;
        m_currentSection = logicalIndex;
        m_connections.Clear();
        m_widget->SetHeaderView(tv, params);
        auto x = tv->sectionPosition(logicalIndex);
        auto w = tv->sectionSize(logicalIndex);
        auto h = tv->height();
        auto sizeHint = this->sizeHint();
        auto px = x + w - sizeHint.width() - 5;
        auto py = (h - sizeHint.height()) / 2;
        setParent(tv);
        move(px,py);
        show();
        m_connections.connect(tv->parentWidget(), &QWidget::destroyed, [this]{
            m_blockHide = false;
            Hide();
        });
    }

    void Hide()
    {
        if(m_blockHide) {
            return;
        }
        m_current = nullptr;
        m_currentSection = -1;
        setParent(nullptr);
        m_widget->SetHeaderView(nullptr, DescTableViewParams());
        hide();
    }

private:
    WidgetsResizableHeaderAttachmentShowColumnsWidget* m_widget;
    QtLambdaConnections m_connections;
    QHeaderView* m_current;
    qint32 m_currentSection;
    bool m_blockHide;
};

WidgetsResizableHeaderAttachment::WidgetsResizableHeaderAttachment(Qt::Orientation orientation, QTableView* parent, const DescTableViewParams& params)
    : Super(orientation, parent)
{
    qint32 alignment = Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap;
    setDefaultAlignment((Qt::Alignment)alignment);
    setStretchLastSection(params.StretchLastSection);
    setSectionsMovable(params.SectionsMovable);
    setSectionsClickable(params.SectionsClickable);
    setHighlightSections(params.HighlightSections);

    setDropIndicatorShown(params.DropIndicatorShown);
    setSortIndicatorShown(params.SortIndicatorShown);
    setMinimumSectionSize(16);
}

WidgetsResizableHeaderAttachment::~WidgetsResizableHeaderAttachment()
{

}

QMenu* WidgetsResizableHeaderAttachment::CreateShowColumsMenu(QHeaderView* hv, const DescTableViewParams& params)
{
    QTableView* table = qobject_cast<QTableView*> (hv->parentWidget());
    Q_ASSERT(table != nullptr);
    hv->setMouseTracking(true);
    WidgetWrapper(hv->viewport()).AddEventFilter([hv, params](QObject*, QEvent* e) {
        static auto* hoverButton = new WidgetsResizableHeaderAttachmentShowColumnsButton;
        if(e->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(e);
            auto logicalIndex = hv->logicalIndexAt(me->pos());
            hoverButton->Show(hv, logicalIndex, params);
        } else if(e->type() == QEvent::Leave) {
            if(qApp->widgetAt(QCursor::pos()) != hoverButton) {
                hoverButton->Hide();
            }
        }
        return false;
    });
    auto* result = MenuWrapper(table).AddPreventedFromClosingMenu(hv->orientation() == Qt::Horizontal ? tr("Show Columns") : tr("Show Rows"));
    connect(result, &QMenu::aboutToShow, [table, result, params, hv]{
        result->clear();
        MenuWrapper wrapper(result);

        auto* model = table->model();
        for(int i=0; i < hv->count(); ++i){
            auto foundIt = params.ColumnsParams.find(i);
            if(foundIt != params.ColumnsParams.end()){
                if(!foundIt->CanBeHidden) {
                    continue;
                }
            }
            auto title = model->headerData(i, hv->orientation()).toString();
            wrapper.AddCheckboxAction(title, !hv->isSectionHidden(i), [hv, i](bool checked){
                hv->setSectionHidden(i, !checked);
            });
        }
        auto oldActions = result->actions();
        wrapper.AddAction(tr("Select All"), [oldActions]{
            for(auto* action : oldActions) {
                if(!action->isChecked()) {
                    action->trigger();
                }
            }
        });
        wrapper.AddAction(tr("Deselect All"), [oldActions]{
            for(auto* action : oldActions) {
                if(action->isChecked()) {
                    action->trigger();
                }
            }
        });
        wrapper.AddAction(tr("Swap Selections"), [oldActions]{
            for(auto* action : oldActions) {
                action->trigger();
            }
        });
    });
    return result;
}

QSize WidgetsResizableHeaderAttachment::sectionSizeFromContents(int logicalIndex) const
{
    if(model() != nullptr && model()->rowCount() != 0) {
        auto headerText = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
        auto options = viewOptions();
        auto metrics = QFontMetrics(options.font);
        int margin = style()->pixelMetric(QStyle::PM_HeaderMargin, &options, this);
        margin += style()->pixelMetric(QStyle::PM_HeaderGripMargin, &options, this);
        qint32 maxWidth;
        if(orientation() == Qt::Horizontal) {
            maxWidth = sectionSize(logicalIndex);
        } else {
            maxWidth = width();
        }
        auto rect = metrics.boundingRect(QRect(margin, margin, maxWidth - margin, 5000), defaultAlignment(), headerText);
        return rect.size() + QSize(margin * 2, margin * 2);
    }

    return Super::sectionSizeFromContents(logicalIndex);
}

void WidgetsResizableHeaderAttachment::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    Super::paintSection(painter, rect, logicalIndex);
}

#include "widgetsresizableheaderattachment.moc"
