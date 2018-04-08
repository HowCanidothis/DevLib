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
#include "SharedGui/gt_decl.h"

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
            QRect row_rect(0,option.rect.y(),option.widget->width(),orect.height());
            QLinearGradient lg(0,row_rect.y(), row_rect.width(),row_rect.y());
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
        const QVariant& data = index.data(Qt::EditRole);
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
};

PropertiesView::PropertiesView(QWidget* parent, Qt::WindowFlags flags)
    : Super(parent)
    , text_editor("Common/Text editor", "C:\\Windows\\system32\\notepad.exe")
{
    setWindowFlags(windowFlags() | flags);
    setItemDelegate(new PropertiesDelegate(this));
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    header()->hide();
    setIndentation(5);
    setAnimated(true);

    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    properties_model = new PropertiesModel(this);
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(properties_model);
    setModel(proxy);

    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    auto addAction = [&](const QString& name, const QString& tr){
        QAction* action = new QAction(tr, this);
        this->addAction(action);
        action->setObjectName(name);
        return action;
    };

    action_open_with_text_editor = addAction("OpenWithTextEditor", tr("Open with text editor"));

    setContextMenuPolicy(Qt::ActionsContextMenu);

    QMetaObject::connectSlotsByName(this);
}

void PropertiesView::save(const QString& file_name)
{
    properties_model->save(file_name);
}

void PropertiesView::load(const QString& file_name)
{
    properties_model->load(file_name);
}

void PropertiesView::showEvent(QShowEvent*)
{
    if(!model()->rowCount()) {
        properties_model->update();
    }
}

void PropertiesView::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::RightButton) {
        index_under_cursor = this->indexAt(event->pos());
        validateActionsVisiblity();
    }
    QAbstractItemView::State preState = state();
    QTreeView::mouseReleaseEvent(event);
    if (preState == QAbstractItemView::AnimatingState)
        setState(preState);
}

void PropertiesView::validateActionsVisiblity()
{
    if(index_under_cursor.data(PropertiesModel::RoleIsTextFileName).toBool()) {
        action_open_with_text_editor->setVisible(true);
    }
    else {
        action_open_with_text_editor->setVisible(false);
    }
}

void PropertiesView::on_OpenWithTextEditor_triggered()
{
    LOGOUT;
    QString open_file = index_under_cursor.data().toString();

    QStringList arguments { open_file };

    QProcess *myProcess = new QProcess(this);
    myProcess->start(text_editor, arguments);
}

#include "propertiesview.moc"
