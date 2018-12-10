#include "propertiesdialogbase.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QApplication>

PropertiesDialogBase::PropertiesDialogBase(const QString& name, qint32 contextIndex, QWidget* view, QWidget* parent)
    : QDialog(parent)
    , _isInitialized(false)
    , _options(Options_Default)
    , _contextIndex(contextIndex)
    , _view(view)
    , _savedGeometry(Name("PropertiesDialogGeometry/" + name), PropertiesSystem::Global)
{
    if(!_savedGeometry.IsValid()) {
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

    _oldValues.clear();

    changeProperties([this,propertiesInitializeFunction]{
        PropertiesSystem::Begin(_contextIndex);
        propertiesInitializeFunction();
        if(_options.TestFlag(Option_ReadOnly)) {
            PropertiesSystem::ForeachProperty([](Property* property){
                property->ChangeOptions().AddFlag(Property::Option_IsReadOnly);
            });
        } else {
            PropertiesSystem::ForeachProperty([this](Property* property){
                if(property->GetOptions().TestFlag(Property::Option_IsPresentable)) {
                    property->Subscribe([this, property]{
                        auto find = _oldValues.find(property);
                        if(find == _oldValues.end()) {
                            _oldValues.insert(property, property->GetPreviousValue());
                        }
                    });
                }
            });
        }
        PropertiesSystem::End();
    });

    _isInitialized = true;
}

void PropertiesDialogBase::SetOnDone(const PropertiesDialogBase::OnDoneHandle& onDone)
{
    disconnect(_connection);
    _connection = connect(this, &QDialog::finished, onDone);
}

void PropertiesDialogBase::done(int result)
{
    Q_ASSERT(_isInitialized == true);
    if(_savedGeometry.IsValid()) {
        _savedGeometry = saveGeometry();
    }
    Super::done(result);
    if(result == Rejected) {
        auto it = _oldValues.begin();
        auto e = _oldValues.end();
        for(; it != e; it++) {
            it.key()->SetValue(it.value());
        }
    }

    if(_options.TestFlag(Option_ClearContextOnDone)) {
        PropertiesSystem::Clear(_contextIndex);
    }
    _isInitialized = false;
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
