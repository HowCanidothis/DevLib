#include "propertiesdialogbase.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QApplication>

PropertiesDialogBase::PropertiesDialogBase(const QString& name, qint32 contextIndex, QWidget* view, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , _contextIndex(contextIndex)
    , _view(view)
    , _savedGeometry(Name("PropertiesDialogGeometry/" + name), PropertiesSystem::Global)
{
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(view);
    layout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void PropertiesDialogBase::Initialize(const PropertiesDialogBase::StdHandle& propertiesInitializeFunction, const PropertiesDialogBase::StdHandle& onChange, bool isReadOnly)
{
    if(isVisible()) {
        qApp->setActiveWindow(this);
        return;
    }

    _isDirty = false;

    changeProperties([this,onChange,isReadOnly,propertiesInitializeFunction]{
        PropertiesSystem::Begin(_contextIndex);
        propertiesInitializeFunction();
        if(isReadOnly) {
            PropertiesSystem::ForeachProperty([](Property* property){
                property->ChangeOptions().AddFlag(Property::Option_IsReadOnly);
            });
        } else {
            PropertiesSystem::Subscribe([this, onChange] {
                onChange();
                _isDirty = true;
            });
        }
        PropertiesSystem::End();
    });
}

void PropertiesDialogBase::SetOnDone(const PropertiesDialogBase::OnDoneHandle& onDone)
{
    disconnect(_connection);
    _connection = connect(this, &QDialog::finished, onDone);
}

void PropertiesDialogBase::done(int result)
{
    if(_savedGeometry.IsValid()) {
        _savedGeometry = saveGeometry();
    }
    Super::done(result);
    PropertiesSystem::Clear(_contextIndex);
}

void PropertiesDialogBase::showEvent(QShowEvent* event)
{
    if(_savedGeometry.IsValid() && !_savedGeometry.Native().isEmpty()) {
        restoreGeometry(_savedGeometry);
    } else {
        Super::showEvent(event);
    }
}

void PropertiesDialogBase::changeProperties(const PropertiesDialogBase::StdHandle& changingProperties)
{
    changingProperties();
}
