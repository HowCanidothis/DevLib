#include "commandsvisualcomponents.h"

#include <QMouseEvent>
#include <QAction>
#include <QItemSelectionModel>

CommandsModel::CommandsModel(CommandsContainerVisual* cmds)
    : _commands(cmds)
{
    connect(_commands, SIGNAL(containerAboutToBeChanged()), this, SIGNAL(modelAboutToBeReset()));
    connect(_commands, SIGNAL(containerChanged()), this, SIGNAL(modelReset()));
}

void CommandsModel::Execute(qint32 from, qint32 to)
{
    if(from != to) {
        if(from < to) {
            while(from++ != to) {
                _commands->Redo();
            }
        }
        else {
            while(from-- != to) {
                _commands->Undo();
            }
        }
    }
}

void CommandsModel::SetSource(CommandsContainerVisual* cmds)
{
    emit layoutAboutToBeChanged();
    _commands = cmds;
    emit layoutChanged();
    connect(_commands, SIGNAL(containerAboutToBeChanged()), this, SIGNAL(modelAboutToBeReset()));
    connect(_commands, SIGNAL(containerChanged()), this, SIGNAL(modelReset()));
}

QModelIndex CommandsModel::GetCurrentIndex() const
{
    return index(_commands->GetCurrentIndex() + 1);
}

int CommandsModel::rowCount(const QModelIndex&) const
{
    return _commands->GetCount() + 1;
}

QVariant CommandsModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        if(index.row()) {
            capacity_type commandIndex = static_cast<capacity_type>(index.row() - 1);
            return QString::number(index.row()) + "-" + _commands->GetCommand(commandIndex)->GetText();
        } else {
            return tr("<empty>");
        }
    default:
        break;
    }
    return QVariant();
}

CommandsView::CommandsView(QWidget* p)
    : QListView(p)
{
    setSelectionMode(CommandsView::SingleSelection);

    this->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void CommandsView::SetModel(CommandsModel* cmd_model)
{
    QItemSelectionModel* m = this->selectionModel();
    this->setModel(cmd_model);
    delete m;

    QModelIndex currentIndex = cmd_model->GetCurrentIndex();
    _lIndex = currentIndex.row();
    this->setCurrentIndex(currentIndex);

    connect(cmd_model->_commands, SIGNAL(currentIndexChanged(qint32)), this, SLOT(onCurrentIndexChanged(qint32)));
    connect(this->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
}

CommandsModel* CommandsView::GetModel() const
{
    return static_cast<CommandsModel*>(model());
}

void CommandsView::setModel(QAbstractItemModel* model)
{
    QListView::setModel(model);
}

void CommandsView::onCurrentIndexChanged(qint32 v)
{
    _lIndex = v + 1;
    this->setCurrentIndex(model()->index(v + 1,0));
}

void CommandsView::onSelectionChanged(const QItemSelection& selected, const QItemSelection&)
{
    if(!selected.isEmpty()){
        QModelIndex selmi = selected.indexes().first();
        if(selmi.isValid()){
            GetModel()->Execute(_lIndex, selmi.row());
            _lIndex = selmi.row();
        }
    }
}

void CommandsView::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton) {
        QListView::mousePressEvent(e);
    }
}

void CommandsContainerVisual::AddCommand(CommandBase* cmd)
{
    if(_currentMacro) {
        _currentMacro->Push(cmd);
    }
    else {
        emit containerAboutToBeChanged();
        addCommandPrivate(cmd);
        emit containerChanged();
        emit currentIndexChanged(GetCurrentIndex());
    }
}

void CommandsContainerVisual::AddCommandAndDo(CommandBase* cmd)
{
    AddCommand(cmd);
    cmd->Redo();
}

void CommandsContainerVisual::Clear()
{
    emit containerAboutToBeChanged();
    CommandsContainer::Clear();
    emit containerChanged();
}

void CommandsContainerVisual::Redo()
{
    if(CanRedo()) {
        redoPrivate();
        emit currentIndexChanged(GetCurrentIndex());
    }
}

void CommandsContainerVisual::Undo()
{
    if(CanUndo()) {
        undoPrivate();
        emit currentIndexChanged(GetCurrentIndex());
    }
}
