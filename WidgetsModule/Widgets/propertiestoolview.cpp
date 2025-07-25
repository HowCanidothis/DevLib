#include "propertiestoolview.h"

#include <QGridLayout>
#include <QTextEdit>
#include <WidgetsModule/internal.hpp>

const SerializerVersion stateVersion(0x9609e641f01de6, 0,0);

template<>
struct Serializer<QHash<Name, PropertiesToolFolderView::BindingRules>>
{
    using TypeName = QHash<Name, PropertiesToolFolderView::BindingRules>;

    template<class Buffer>
    static void Write(Buffer& buffer, const TypeName& data)
    {
        int count = data.size();
        buffer.BeginKeyValueArray(buffer, count);
        for(auto it = data.begin(); it != data.end(); ++it){
            buffer.KeyValue(buffer, it.key(), WidgetAbstractButtonWrapper(it.value().ToolButton).WidgetChecked().Native());
        }
    }

    template<class Buffer>
    static void Read(Buffer& buffer, TypeName& data)
    {
        int count = data.size();
        buffer.BeginKeyValueArray(buffer, count);
        while(count--){
            Name key; bool expand;
            buffer.KeyValue(buffer, key, expand);
            auto it = data.find(key);
            if(it != data.end()){
                WidgetAbstractButtonWrapper(it.value().ToolButton).WidgetChecked() = expand;
            }
        }
    }
};

struct PropertiesToolFolderViewButtonAttachment
{

};

PropertiesToolFolderView::PropertiesToolFolderView(QWidget* parent)
    : Super(parent)
    , m_deleteButtonOverFolder(nullptr)
{
    setLayout(m_layout = new QVBoxLayout);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(1);
    WidgetPushButtonWrapper(m_deleteButton = new QPushButton(this)).SetControl(ButtonRole::Icon).SetIcon(ActionIcons::Delete);
    m_deleteButton->hide();
}

void PropertiesToolFolderView::Bind()
{
    for(const auto& binding : m_widgets) {
        if(binding.Binding != nullptr) {
            binding.Binding();
        }
    }
}

PropertiesToolViewWrapper PropertiesToolFolderView::BeginFolder(const Name& folderName, const FTranslationHandler& title, int index)
{
    auto* widget = new PropertiesToolView();
    widget->ForceDisabled.ConnectFrom(CDL, ForceDisabled).MakeSafe(widget->m_connections);
    OnAboutToBeChanged.ConnectFrom(CDL, widget->OnAboutToBeChanged);
    widget->Key = folderName;
    auto buttonWrapper = BeginFolder(folderName, title, BindingRules(widget, [widget]{ widget->Bind(); }, [widget]{ widget->ClearBindings(); }), index);
    WidgetPropertiesToolViewWrapper folderWrapper(widget);
    buttonWrapper.WidgetVisibility() = true;
    folderWrapper.WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, [buttonWrapper](bool checked, bool visible){
        return checked && visible;
    }, buttonWrapper.WidgetChecked(), buttonWrapper.WidgetVisibility());
    auto b = buttonWrapper.GetWidget()->text();
    return {buttonWrapper, folderWrapper};
}

WidgetAbstractButtonWrapper PropertiesToolFolderView::BeginFolder(const Name& folderName, const FTranslationHandler& title, const BindingRules& widget, int index)
{
    index = index < 0 ? Count() : index;
    Q_ASSERT(!m_widgets.contains(folderName));
    WidgetAbstractButtonWrapper buttonWrapper(new QPushButton);
    buttonWrapper->setCheckable(true);
    buttonWrapper->setProperty("PropertiesToolBarButton", true);
    buttonWrapper->setObjectName(folderName.AsString() + "Button");
    buttonWrapper.WidgetText()->SetTranslationHandler(title);
    buttonWrapper.WidgetChecked().ConnectAndCall(CONNECTION_DEBUG_LOCATION, [buttonWrapper](bool check){
        buttonWrapper.SetIcon(check ? "CollapseIcon" : "ExpandIcon");
    });

    const_cast<BindingRules&>(widget).ToolButton = buttonWrapper;

    widget.Widget->setObjectName(folderName.AsString());
    m_layout->insertWidget(index, buttonWrapper);
    auto* headerItem = m_layout->itemAt(index++);
    m_layout->insertWidget(index, widget.Widget);
    auto* bodyItem = m_layout->itemAt(index);

    ///TODO
    const_cast<BindingRules&>(widget).Destroy = [this, headerItem, bodyItem]{
        m_layout->removeItem(headerItem);
        m_layout->removeItem(bodyItem);
        delete headerItem;
        delete bodyItem;
    };
    m_widgets.insert(folderName, widget);

    return buttonWrapper;
}

void PropertiesToolFolderView::Clear()
{
    for(const auto& rule : m_widgets) {
        rule.Destroy();
        rule.Widget->deleteLater();
        rule.ToolButton->deleteLater();
    }
    m_widgets.clear();
}

bool PropertiesToolFolderView::DestroyFolder(const Name& folderName){
    auto bindingIter = m_widgets.find(folderName);
    if(bindingIter == m_widgets.end()){
        return false;
    }
    auto rule = bindingIter.value();
    rule.Destroy();
    rule.Widget->deleteLater();
    rule.ToolButton->deleteLater();
    m_widgets.remove(folderName);
    return true;
}

int PropertiesToolFolderView::Count() {
    return m_layout->count();
}

LineData PropertiesToolView::AddColorProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyColor* ()>& propertyGetter, bool hasAlpha)
{
    auto* cb = new WidgetsColorPicker();
    cb->HasAlpha = hasAlpha;

    return addProperty(propertyName, title, cb, [this, propertyGetter, cb](QWidget*){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        m_connectors.AddConnector<LocalPropertiesPushButtonConnector>(property, cb);
    });
}

DispatcherConnection PropertiesToolFolderView::AddDeleteButton(const WidgetAbstractButtonWrapper& folder, const FAction& onClicked)
{
    auto visible = ::make_shared<LocalPropertyBool>();
    auto delay = DelayedCallObjectCreate();
    visible->ConnectAndCall(CDL, [this, delay](bool visible){
        delay->Call(CDL, [this, visible]{
            if(visible) {
                m_deleteButton->show();
                m_deleteButton->raise();
            } else {
                m_deleteButton->hide();
            }
        });
    });
    WidgetWrapper(m_deleteButton).AddEventFilter([visible](QObject*, QEvent* e) {
        switch(e->type()) {
        case QEvent::HoverEnter:
            *visible = true;
        default: break;
        }
        return false;
    });

    folder.AddEventFilter([this, folder, visible](QObject*, QEvent* e) {
        switch(e->type()) {
        case QEvent::HoverEnter: 
            m_deleteButton->move(QPoint(folder->x() + folder->width() - m_deleteButton->width() - m_buttonOffset.width(), folder->y() + m_buttonOffset.height()));
            m_deleteButtonOverFolder = folder;
            m_deleteButton->show();
            m_deleteButton->raise();
            *visible = true;
            break;
        case QEvent::HoverLeave:
            *visible = false;
            break;
        default: break;
        }
        return false;
    });
    return WidgetAbstractButtonWrapper(m_deleteButton).OnClicked().Connect(CDL, [this, onClicked, folder]{
        if(m_deleteButtonOverFolder == folder.GetWidget()) {
            if(WidgetsDialogsManager::GetInstance().ShowTempDialog(DescCustomDialogParams().SetTitle(TR(tr("Delete Stabilizer?")))
            .AddButtons(WidgetsDialogsManagerDefaultButtons::CancelButton(),
                        WidgetsDialogsManagerDefaultButtons::DiscardRoleButton(TR(tr("Delete")))))) {
                onClicked();
                m_deleteButton->hide();
            }
        }
    });
}

void PropertiesToolFolderView::AddWidget(QWidget* w, int index)
{
    m_layout->insertWidget(index, w);
}

void PropertiesToolFolderView::AddLayout(QLayout* l, int index)
{
    m_layout->insertLayout(index, l);
}

void PropertiesToolFolderView::Release()
{
    for(const auto& binding : m_widgets) {
        if(binding.ReleaseBind != nullptr) {
            binding.ReleaseBind();
        }
    }
}

QByteArray PropertiesToolFolderView::SaveState() const
{
    return SerializeToArrayVersioned(stateVersion, m_widgets, SerializationMode_InvokeProperties);
}

bool PropertiesToolFolderView::LoadState(const QByteArray& state)
{
    return DeSerializeFromArrayVersioned(stateVersion, state, m_widgets, SerializationMode_InvokeProperties).first;
}

PropertiesToolView::PropertiesToolView(QWidget* parent)
    : Super(parent)
{
    setLayout(m_layout = new QGridLayout);
    m_layout->addItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0, 1, 2);
    m_layout->setContentsMargins(0,6,0,6); // TODO.
    m_layout->setSpacing(6);
    m_connectors.ForceDisabled.ConnectFrom(CDL, ForceDisabled);
    OnAboutToBeChanged.ConnectFrom(CDL, m_connectors.OnAboutToBeChanged);
}

LineData PropertiesToolView::AddProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyString* ()>& propertyGetter)
{
    auto* cb = new QComboBox;
    cb->setEditable(true);

    return addProperty(propertyName, title, cb, [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* comboBox = reinterpret_cast<QComboBox*>(w);
        m_connectors.AddConnector<LocalPropertiesLineEditConnector>(property, comboBox->lineEdit());
    });
}

LineData PropertiesToolView::AddProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyBool* ()>& propertyGetter)
{
    return addProperty(propertyName, title, new QCheckBox, [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* checkBox = reinterpret_cast<QCheckBox*>(w);
        m_connectors.AddConnector<LocalPropertiesCheckBoxConnector>(property, checkBox);
    });
}

LineData PropertiesToolView::AddTextProperty(const Name& propertyName, const FTranslationHandler& title, const std::function<LocalPropertyString* ()>& propertyGetter)
{
    auto binding = [this, propertyGetter](QWidget* w){
        auto* property = propertyGetter();
        if(property == nullptr) {
            return;
        }
        auto* lineEdit = reinterpret_cast<WidgetsTextEditLayout*>(w);
        m_connectors.AddConnector<LocalPropertiesTextEditConnector>(property, lineEdit->textEdit());
    };

    Q_ASSERT(!m_bindings.contains(propertyName));
    auto* edit = new WidgetsTextEditLayout();
    edit->setTitle(title());
    auto lineData = AddData(propertyName, edit, nullptr);
    m_bindings.insert(propertyName, { lineData, binding });
    return lineData;
}
#ifdef UNITS_MODULE_LIB
LineData PropertiesToolView::AddDoubleProperty(const Name& propertyName, const Measurement* measurement, const FTranslationHandler& title, const std::function<LocalPropertyDoubleOptional* ()>& propertyGetter, const QVector<Dispatcher*>& labelUpdaters)
{
    return AddProperty<LocalPropertyDoubleOptional>(propertyName, measurement, title, propertyGetter, labelUpdaters);
}
#endif

const LineData& PropertiesToolView::FindRow(const Name& propertyName) const
{
    auto iter = m_bindings.find(propertyName);
    Q_ASSERT(iter != m_bindings.end());
    return iter.value().Data;
}

LineData PropertiesToolView::BeginGroup(const FTranslationHandler& header)
{
    int rowCount = m_layout->rowCount();
    auto* label = new QLabel(header());
    m_layout->addWidget(label, rowCount, 0, 1, 2);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setProperty("GroupTitle", true);
    return LineData(Name(), label);
}

LineData PropertiesToolView::AddTableView(const Name& id, ViewModelsTableBase* viewModel, const Latin1Name& stateTag)
{
    auto* tv = new WidgetsAdjustableTableView(this);
    tv->setModel(viewModel);
    viewModel->ForceDisabled.ConnectFrom(CDL, ForceDisabled);
    tv->KeepStrictWidth = false;
    WidgetTableViewWrapper(tv).InitializeHorizontal(DescTableViewParams().SetStateTag(stateTag));
    auto handlers = WidgetsGlobalTableActionsScope::AddDefaultHandlers(tv);
    handlers->ShowAll();
    handlers->IsReadOnly.ConnectFrom(CDL, ForceDisabled);
    return AddData(id, tv);
}

LineData PropertiesToolView::AddData(const Name& id, QWidget* widget, const FTranslationHandler& title, const QVector<Dispatcher*>& labelUpdaters, Qt::Orientation orientation)
{
    widget->setObjectName(id.AsString());
    int rowCount = m_layout->rowCount();
    if(title == nullptr) {
        m_layout->addWidget(widget, rowCount, 0, 1, 2);
        LineData result (id, widget);
        OnPropertyAdded(result);
        return result;
    }

    auto header = new ElidedLabel(this);
    LineData result(id, widget, header);
    result.LabelWrapper.WidgetText()->SetTranslationHandler(CDL, title, labelUpdaters);

    switch(orientation){
    case Qt::Horizontal: {
        header->setProperty("HorizontalLabel", true);
        widget->setFixedWidth(150);
        m_layout->addWidget(header, rowCount, 0);
        m_layout->addWidget(widget, rowCount, 1);
        break;
    }
    case Qt::Vertical: {
        header->setAlignment(Qt::AlignCenter);
        m_layout->addWidget(header, rowCount++, 0, 1, 2);
        m_layout->addWidget(widget, rowCount, 0, 1, 2);
        break;
    }};

    auto* cb = qobject_cast<QComboBox*>(widget);
    if(cb != nullptr) {
        WidgetWrapper(cb->view()).ConnectFocus(header);
    }
    auto& widgets = WidgetWrapper(widget).WidgetTrueFocusWidgets();
    if(widgets.isEmpty()) {
        WidgetWrapper(widget).ConnectFocus(header);
    } else {
        for(auto* w : widgets) {
            WidgetWrapper(w).ConnectFocus(header);
        }
    }
    OnPropertyAdded(result);
    return result;
}

void PropertiesToolView::Bind()
{
    ClearBindings();
    for(const auto& binding : m_bindings) {
        binding.BindingHandler(binding.Data.EditorWrapper.GetWidget());
    }
}

void PropertiesToolView::ClearBindings()
{
    m_connectors.Clear();
}

LineData PropertiesToolView::addProperty(const Name& propertyName, const FTranslationHandler& title, QWidget* widget, const std::function<void (QWidget*)>& binding){
    Q_ASSERT(!m_bindings.contains(propertyName));
    auto lineData = AddData(propertyName, widget, title);
    m_bindings.insert(propertyName, { lineData, binding });
    return lineData;
}

WidgetPropertiesToolViewWrapper::WidgetPropertiesToolViewWrapper(PropertiesToolView* folder)
    : Super(folder)
{

}

LocalPropertyBoolCommutator& WidgetPropertiesToolViewWrapper::ContentVisibilityCommutator() const
{
    auto toolView = GetWidget();
    auto value = toolView->property("a_content").value<SharedPointer<LocalPropertyBoolCommutator>>();
    if(value == nullptr) {
        value = new LocalPropertyBoolCommutator();
        toolView->setProperty("a_content", QVariant::fromValue(value));
    }
    return *value;
}

LineData::LineData()
    : Label(nullptr)
    , Editor(nullptr)
    , EditorWrapper(Editor)
    , LabelWrapper(Label)
{

}

LineData::LineData(const Name& id, QWidget* w, ElidedLabel* lb)
    : Label(lb)
    , Editor(w)
    , EditorWrapper(w)
    , LabelWrapper(lb)
    , m_id(id)
{
    if(lb != nullptr) {
//#ifdef QT_DEBUG
//        auto& connected = *EditorWrapper.Injected<bool>("LineDataConnected", []{ return new bool(false); });
//        Q_ASSERT(connected == false);
//        connected = true;
//#endif
        LabelWrapper.ConnectVisibilityFrom(CONNECTION_DEBUG_LOCATION, EditorWrapper);
    }
}

//LocalPropertyErrorsContainer& LineData::WidgetErrors() const {
//    return EditorWrapper.WidgetErrors();
//}

LocalPropertyBoolCommutator& LineData::ContentVisibilityCommutator() const
{
    auto value = Editor->property("a_content").value<SharedPointer<LocalPropertyBoolCommutator>>();
    if(value == nullptr) {
        value = new LocalPropertyBoolCommutator(LocalPropertyBoolCommutator::And);
        Editor->setProperty("a_content", QVariant::fromValue(value));
        WidgetVisibility().ConnectFrom(CONNECTION_DEBUG_LOCATION, [](bool v){ return v; }, *value);
    }
    return *value;
}

LocalPropertyBoolCommutator& PropertiesToolViewWrapper::ContentVisibilityCommutator() const
{
    return FolderWrapper.ContentVisibilityCommutator();
}
