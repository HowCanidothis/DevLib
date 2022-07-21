#include "propertiesdialogbase.h"

#ifdef QT_GUI_LIB

#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QApplication>

#include "propertiesscope.h"

PropertiesDialogBase::PropertiesDialogBase(const Name& scope, QWidget* view, QWidget* parent)
    : QDialog(parent)
    , m_isInitialized(false)
    , m_options(Options_Default)
    , m_scope(scope)
    , m_view(view)
{
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(view);
    layout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    currentDialogs().insert(this);
}

PropertiesDialogBase::~PropertiesDialogBase()
{
    currentDialogs().remove(this);
}

void PropertiesDialogBase::RejectAllDialogs()
{
    for(auto* dialog : currentDialogs()) {
        if(dialog->isVisible()) {
            dialog->reject();
        }
    }
}

void PropertiesDialogBase::Initialize(const PropertiesDialogBase::StdHandle& propertiesInitializeFunction)
{
    if(isVisible()) {
        qApp->setActiveWindow(this);
        return;
    }

    m_oldValues.clear();

    changeProperties([this,propertiesInitializeFunction]{
        PropertiesSystem::Begin(m_scope);
        propertiesInitializeFunction();
        if(m_options.TestFlag(Option_ReadOnly)) {
            PropertiesSystem::ForeachProperty([](Property* property){
                property->ChangeOptions().AddFlag(Property::Option_IsReadOnly);
            });
        } else {
            PropertiesSystem::ForeachProperty([this](Property* property){
                if(property->GetOptions().TestFlag(Property::Option_IsPresentable)) {
                    property->GetDispatcher().Connect(CONNECTION_DEBUG_LOCATION, [this, property]{
                        auto find = m_oldValues.find(property);
                        if(find == m_oldValues.end()) {
                            m_oldValues.insert(property, property->GetPreviousValue());
                        }
                    }).MakeSafe(m_additonalPropertiesConnections);
                }
            });
        }
        PropertiesSystem::End();
    });

    m_isInitialized = true;
}

void PropertiesDialogBase::SetOnDone(const PropertiesDialogBase::OnDoneHandle& onDone)
{
    disconnect(m_connection);
    m_connection = connect(this, &QDialog::finished, onDone);
}

void PropertiesDialogBase::done(int result)
{
    Q_ASSERT(m_isInitialized == true);
    Super::done(result);
    if(result == Rejected) {
        auto it = m_oldValues.begin();
        auto e = m_oldValues.end();
        for(; it != e; it++) {
            it.key()->SetValue(it.value());
        }
    }

    m_additonalPropertiesConnections.clear();

    if(m_options.TestFlag(Option_ClearContextOnDone)) {
        PropertiesSystem::Clear(m_scope);
    }
    m_isInitialized = false;
}

void PropertiesDialogBase::showEvent(QShowEvent* event)
{
    Super::showEvent(event);
}

void PropertiesDialogBase::changeProperties(const PropertiesDialogBase::StdHandle& changingProperties)
{
    changingProperties();
}

QSet<PropertiesDialogBase*>& PropertiesDialogBase::currentDialogs()
{
    static QSet<PropertiesDialogBase*> result;
    return result;
}

#endif
