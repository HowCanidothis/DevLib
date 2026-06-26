#include "topnotifierframe.h"
#include "ui_topnotifierframe.h"

#include <QResizeEvent>
#include <QLineEdit>

#include "WidgetsModule/Utils/widgethelpers.h"
#include "WidgetsModule/Attachments/widgetslocationattachment.h"
#include "WidgetsModule/Components/componentplacer.h"

TopNotifierFrame::TopNotifierFrame(QWidget* parent)
    : Super(parent)
    , ui(new Ui::TopNotifierFrame)
{
    setProperty("use_preffered_size", QSize(50000, 100));
    ui->setupUi(this);

    m_placer = WidgetWrapper(this).LocateToParent(DescWidgetsLocationAttachmentParams(QuadTreeF::Location_MiddleTop).SetOffset({0,0}));

    hide();

    ui->Label->setTextFormat(Qt::RichText);
    ui->Label->setOpenExternalLinks(false);

    setWindowTitle(QString());

    WidgetLabelWrapper(ui->Label).OnLinkActivated().Connect(CONNECTION_DEBUG_LOCATION, [this](const Name& link){
        (*(FAction*)link.AsString().toLongLong())();
    });

    setFocusPolicy(Qt::NoFocus);
}

TopNotifierFrame::~TopNotifierFrame()
{
    delete ui;
}

const TranslatedStringPtr& TopNotifierFrame::WidgetText()
{
    return WidgetLabelWrapper(ui->Label).WidgetText();
}

LocalProperty<QuadTreeF::BoundingRect_Location>& TopNotifierFrame::WidgetLocation()
{
    return m_placer->GetComponentPlacer()->Location;
}

LocalProperty<QPoint>& TopNotifierFrame::WidgetOffset()
{
    return m_placer->GetComponentPlacer()->Offset;
}

TopNotifierFrameErrorsComponent::TopNotifierFrameErrorsComponent(TopNotifierFrame* frame)
    : TopNotifierFrameErrorsComponent(new LocalPropertyErrorsViewModel(), frame)
{
    m_internalErrors = true;
}

TopNotifierFrameErrorsComponent::TopNotifierFrameErrorsComponent(LocalPropertyErrorsViewModel* errors, TopNotifierFrame* frame)
    : Super(frame)
    , m_updateText(1000)
    , m_errors(errors)
    , m_internalErrors(false)
    , m_useDefaultActionHandlers(false)
{
    auto setText = m_updateText.Wrap(CONNECTION_DEBUG_LOCATION, [this, frame, errors]{
        m_defaultActivationHandlers.clear();
        frame->WidgetText()->SetTranslationHandler([this, errors]{
            StringBuilder message;
            for(const Name& error : *errors) {
                const auto& desc = errors->GetDescription(error);
                if(desc.ActivationHandler != nullptr) {
                    message.XMLAddEnumerated(message.XMLCreateHyperlink(desc.Text->Native(), &desc.ActivationHandler));
                } else if(m_useDefaultActionHandlers){
                    auto activationHandler = ::make_shared<FAction>([this, error]{
                        OnErrorActivated()(error);
                    });
                    m_defaultActivationHandlers.append(activationHandler);
                    message.XMLAddEnumerated(message.XMLCreateHyperlink(desc.Text->Native(), activationHandler.get()));
                } else {
                    message.XMLAddEnumerated(desc.Text->Native());
                }
            }
            return message;
        });
        WidgetWrapper(frame).SetVisibleAnimated(!errors->GetModel()->IsEmpty());
    });
    errors->GetModel()->OnChanged.Connect(CONNECTION_DEBUG_LOCATION, setText).MakeSafe(m_connections);
    errors->OnErrorsLabelsChanged.ConnectAndCall(CONNECTION_DEBUG_LOCATION, setText).MakeSafe(m_connections);
    setText();
}

DispatcherConnections TopNotifierFrameErrorsComponent::AddFocusComponent(TopNotifierFrameErrorsFocusComponent* component)
{
#ifdef QT_DEBUG
    Q_ASSERT(!m_focusComponents.contains(component));
#endif
    auto result = component->ConnectFromViewModel(CDL, m_errors);
    result += OnErrorActivated().Connect(CDL, [component](const Name& error){
        component->FocusWidget(error);
    });
#ifdef QT_DEBUG
    m_focusComponents.insert(component);
#endif
    return result;
}

CommonDispatcher<const Name&>& TopNotifierFrameErrorsComponent::OnErrorActivated()
{
    m_useDefaultActionHandlers = true;
    return m_defaultActionHandler;
}

TopNotifierFrameErrorsComponent::~TopNotifierFrameErrorsComponent()
{
    if(m_internalErrors) {
        delete m_errors;
    }
}

void TopNotifierFrameErrorsFocusComponent::updateHighlighted() {
    m_updater.Call(CDL, [this]{
        QHash<Name, TranslatedStringPtr> activeErrors;
        m_collectActiveErrors(activeErrors);
        for(auto iter = m_map.cbegin(); iter != m_map.cend(); ++iter){
            auto errors = iter.value();
            QVector<TranslatedStringPtr> errDescs;
            for(const auto& e : errors) {
                auto foundIt = activeErrors.constFind(e);
                if(foundIt != activeErrors.cend()) {
                    errDescs.append(foundIt.value());
                }
            }
            auto hasError = !errDescs.isEmpty();
            auto* w = reinterpret_cast<QWidget*>(iter.key());
            for(auto* le : w->children()) {
                auto* qle = qobject_cast<QLineEdit*>(le);
                if(qle != nullptr) {
                    WidgetLineEditWrapper(qle).AddWarningIcon(hasError);
                }
            }
            WidgetWrapper(w).WidgetHighlighted() = hasError ? HighLightEnum::Critical : HighLightEnum::None;
            if(hasError) {
                StringBuilder desc;
                for(const auto& error : errDescs) {
                    desc.XMLAddEnumerated(error->Native());
                }
                WidgetWrapper(w).SetToolTip(TR(desc, =));
            } else {
                WidgetWrapper(w).SetToolTip(TR_NONE);
            }
        }
    });
}

TopNotifierFrameErrorsFocusComponent::TopNotifierFrameErrorsFocusComponent(QObject* parent)
    : Super(parent)
{}

TopNotifierFrameErrorsFocusComponent::~TopNotifierFrameErrorsFocusComponent()
{

}

void TopNotifierFrameErrorsFocusComponent::FocusWidget(const Name& focusError)
{
    for(auto it(m_map.cbegin()), e(m_map.cend()); it != e; ++it) {
        if(it.value().contains(focusError)) {
            OnWidgetFocused(focusError, it.key());
            return;
        }
    }
}

DispatcherConnections TopNotifierFrameErrorsFocusComponent::ConnectFromViewModel(const char* cdl, const LocalPropertyErrorsViewModel* model)
{
    DispatcherConnections result;
    result += model->GetModel()->OnChanged.ConnectAndCall(cdl, [this]{
        updateHighlighted();
    });
    result += m_collectActiveErrors.Connect(CDL, [model](QHash<Name, TranslatedStringPtr>& errors) {
        for(const auto& error : *model->GetModel()) {
            errors[error] = model->GetDescription(error).Text;
        }
    });
    return result;
}
