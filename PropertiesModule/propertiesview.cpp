#include "propertiesview.h"

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPainter>
#include <QHeaderView>
#include <QMouseEvent>
#include <QProcess>
#include <QAction>

#include "PropertiesModule/propertiessystem.h"
#include "propertiesmodel.h"
#include "SharedGuiModule/decl.h"

class OnEditorValueChangedListener : public QObject
{
    Q_OBJECT
    QWidget* editor;
    QModelIndex model_index;
    const QStyledItemDelegate* delegate;
public:
    OnEditorValueChangedListener(QWidget* editor, const QModelIndex& model_index, const QStyledItemDelegate* delegate)
        : QObject(editor)
        , editor(editor)
        , model_index(model_index)
        , delegate(delegate)
    {}

public Q_SLOTS:
    void onEditorValueChanged() {
        delegate->setModelData(editor, const_cast<QAbstractItemModel*>(model_index.model()), model_index);
    }
};

class PropertiesDelegate : public QStyledItemDelegate
{
    typedef QStyledItemDelegate Super;
public:
    PropertiesDelegate(QObject* parent)
        : Super(parent)
    {}

    // QAbstractItemDelegate interface
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        if(index.data(PropertiesModel::RoleHeaderItem).toBool()){
            QRect orect = option.rect;
            painter->setPen(Qt::NoPen);
            QRect rowRect(0,option.rect.y(),option.widget->width(),orect.height());
            QLinearGradient lg(0,rowRect.y(), rowRect.width(),rowRect.y());
            lg.setColorAt(0, 0x567dbc);
            lg.setColorAt(0.7, 0x6ea1f1);
            painter->setBrush(lg);
            if(!index.column())
                painter->drawRect(orect.adjusted(-orect.x(),0,0,0));
            else
                painter->drawRect(orect);

            QStyleOptionViewItem opt = option;
            initStyleOption(&opt, index);


            const QWidget *widget = option.widget;

            if(float(option.rect.x()) / widget->width() < 0.5)
                opt.palette.setColor(QPalette::Text,Qt::white);
            else
                opt.palette.setColor(QPalette::Text,Qt::red);
            if(index.column()) {
                opt.text = "";
            }

            widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
        }
        else
            QStyledItemDelegate::paint(painter,option,index);
    }

    QWidget*createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE {
        QVariant data = index.data(Qt::EditRole);
        QVariant delegateValue = index.data(PropertiesModel::RoleDelegateValue);

        if(delegateValue.toInt() == Property::DelegateNamedUInt) {
            QVariant delegateData = index.data(PropertiesModel::RoleDelegateData);
            QComboBox* result = new QComboBox(parent);
            result->addItems(delegateData.toStringList());
            result->setCurrentIndex(data.toUInt());
            return result;
        }

        switch (data.type()) {
        case QVariant::UInt:
        case QVariant::Int: {
            QSpinBox* result = new QSpinBox(parent);
            result->setValue(data.toInt());
            result->setMinimum(index.data(PropertiesModel::RoleMinValue).toInt());
            result->setMaximum(index.data(PropertiesModel::RoleMaxValue).toInt());
            result->setFocusPolicy(Qt::StrongFocus);
            return result;
        }
        case QVariant::Double:
        case QMetaType::Float: {
            QDoubleSpinBox* result = new QDoubleSpinBox(parent);
            result->setValue(data.toDouble());
            result->setMinimum(index.data(PropertiesModel::RoleMinValue).toDouble());
            result->setMaximum(index.data(PropertiesModel::RoleMaxValue).toDouble());
            result->setFocusPolicy(Qt::StrongFocus);
            return result;
        }
        default:
            return Super::createEditor(parent, option, index);
        }
    }

    // QAbstractItemDelegate interface
public:
    void setEditorData(QWidget* editor, const QModelIndex& index) const Q_DECL_OVERRIDE {
        Super::setEditorData(editor, index);
        if(auto e = qobject_cast<QComboBox*>(editor)) {
            auto listener = new OnEditorValueChangedListener(e,index,this);
            connect(e, SIGNAL(currentIndexChanged(int)), listener, SLOT(onEditorValueChanged()));
        }
        else if(auto e = qobject_cast<QSpinBox*>(editor)) {
            auto listener = new OnEditorValueChangedListener(e,index,this);
            connect(e, SIGNAL(valueChanged(int)), listener, SLOT(onEditorValueChanged()));
        }
        else if(auto e = qobject_cast<QDoubleSpinBox*>(editor)) {
            auto listener = new OnEditorValueChangedListener(e,index,this);
            connect(e, SIGNAL(valueChanged(double)), listener, SLOT(onEditorValueChanged()));
        }
    }

    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const Q_DECL_OVERRIDE
    {
        if(auto e = qobject_cast<QComboBox*>(editor)) {
            model->setData(index, e->currentIndex());
        } else {
            Super::setModelData(editor, model, index);
        }
    }
};

static const StringProperty& textEditor(const char* path = nullptr, const char* value = nullptr)
{
    static StringProperty res(path, value);
    return res;
}

PropertiesView::PropertiesView(QWidget* parent, Qt::WindowFlags flags)
    : Super(parent)
{
    textEditor("Common/Text editor", "C:\\Windows\\system32\\notepad.exe");

    setWindowFlags(windowFlags() | flags);
    setItemDelegate(new PropertiesDelegate(this));
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    header()->hide();
    setIndentation(5);
    setAnimated(true);

    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    _propertiesModel = new PropertiesModel(this);
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(_propertiesModel);
    setModel(proxy);

    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    auto addAction = [&](const QString& name, const QString& tr){
        QAction* action = new QAction(tr, this);
        this->addAction(action);
        action->setObjectName(name);
        return action;
    };

    _actionOpenWithTextEditor = addAction("OpenWithTextEditor", tr("Open with text editor"));

    setContextMenuPolicy(Qt::ActionsContextMenu);

    QMetaObject::connectSlotsByName(this);
}

void PropertiesView::Save(const QString& fileName)
{
    _propertiesModel->Save(fileName);
}

void PropertiesView::Load(const QString& fileName)
{
    _propertiesModel->Load(fileName);
}

void PropertiesView::showEvent(QShowEvent*)
{
    if(!model()->rowCount()) {
        _propertiesModel->Update();
    }
}

void PropertiesView::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::RightButton) {
        _indexUnderCursor = this->indexAt(event->pos());
        validateActionsVisiblity();
    }
    QAbstractItemView::State preState = state();
    QTreeView::mouseReleaseEvent(event);
    if (preState == QAbstractItemView::AnimatingState)
        setState(preState);
}

void PropertiesView::validateActionsVisiblity()
{
    if(_indexUnderCursor.data(PropertiesModel::RoleDelegateValue).toInt() == Property::DelegateFileName) {
        _actionOpenWithTextEditor->setVisible(true);
    }
    else {
        _actionOpenWithTextEditor->setVisible(false);
    }
}

void PropertiesView::on_OpenWithTextEditor_triggered()
{
    LOGOUT;
    QString openFile = _indexUnderCursor.data().toString();

    QStringList arguments { openFile };

    QProcess *process = new QProcess(this);
    process->start(textEditor(), arguments);

    log.Warning() << "Opening" << textEditor() << arguments;
}

#include "propertiesview.moc"
