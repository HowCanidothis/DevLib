#include "propertiesdialogbase.h"

#ifdef QT_GUI_LIB

#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QApplication>

PropertiesDialogBase::PropertiesDialogBase(const QString& name, qint32 contextIndex, QWidget* view, QWidget* parent)
    : QDialog(parent)
    , m_isInitialized(false)
    , m_options(Options_Default)
    , m_contextIndex(contextIndex)
    , m_view(view)
    , m_savedGeometry(Name("PropertiesDialogGeometry/" + name), PropertiesSystem::Global)
{
    if(!m_savedGeometry.IsValid()) {
        qCWarning(LC_SYSTEM) << name << "dialog doesn't have geometry property";
    }

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(view);
    layout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void PropertiesDialogBase::CreateGeometryProperty(const QString& dialogName)
{
    // TODO. Memory leak, but this property is global and live until the application close
    auto property = new ByteArrayProperty(Name("PropertiesDialogGeometry/" + dialogName), QByteArray());
    property->ChangeOptions().SetFlags(Property::Option_IsExportable);
}

void PropertiesDialogBase::Initialize(const PropertiesDialogBase::StdHandle& propertiesInitializeFunction)
{
    if(isVisible()) {
        qApp->setActiveWindow(this);
        return;
    }

    m_oldValues.clear();

    changeProperties([this,propertiesInitializeFunction]{
        PropertiesSystem::Begin(m_contextIndex);
        propertiesInitializeFunction();
        if(m_options.TestFlag(Option_ReadOnly)) {
            PropertiesSystem::ForeachProperty([](Property* property){
                property->ChangeOptions().AddFlag(Property::Option_IsReadOnly);
            });
        } else {
            PropertiesSystem::ForeachProperty([this](Property* property){
                if(property->GetOptions().TestFlag(Property::Option_IsPresentable)) {
                    property->Subscribe([this, property]{
                        auto find = m_oldValues.find(property);
                        if(find == m_oldValues.end()) {
                            m_oldValues.insert(property, property->GetPreviousValue());
                        }
                    });
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
    if(m_savedGeometry.IsValid()) {
        m_savedGeometry = saveGeometry();
    }
    Super::done(result);
    if(result == Rejected) {
        auto it = m_oldValues.begin();
        auto e = m_oldValues.end();
        for(; it != e; it++) {
            it.key()->SetValue(it.value());
        }
    }

    if(m_options.TestFlag(Option_ClearContextOnDone)) {
        PropertiesSystem::Clear(m_contextIndex);
    }
    m_isInitialized = false;
}

void PropertiesDialogBase::showEvent(QShowEvent* event)
{
    if(m_savedGeometry.IsValid() && !m_savedGeometry.Native().isEmpty()) {
        restoreGeometry(m_savedGeometry);
    } else {
        Super::showEvent(event);
    }
}

void PropertiesDialogBase::changeProperties(const PropertiesDialogBase::StdHandle& changingProperties)
{
    changingProperties();
}

#endif
