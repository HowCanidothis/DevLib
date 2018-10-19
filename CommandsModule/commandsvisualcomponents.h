#ifndef COMMANDSVISUALCOMPONENTS_H
#define COMMANDSVISUALCOMPONENTS_H

#include <QAbstractListModel>
#include <QListView>

#include "commands.h"

class CommandsContainerVisual : public CommandsContainer
{
    Q_OBJECT
public:
    void AddCommand(CommandBase* cmd);
    void AddCommandAndDo(CommandBase* cmd);

    void Clear();
Q_SIGNALS:
    void containerAboutToBeChanged();
    void containerChanged();
    void currentIndexChanged(qint32 i);
public Q_SLOTS:
    void Redo();
    void Undo();
};

class CommandsView;

class CommandsModel : public QAbstractListModel
{
    // QAbstractItemModel interface

public:
    CommandsModel(CommandsContainerVisual* commands);

    void Execute(qint32 from, qint32 to);
    void SetSource(CommandsContainerVisual* commands);
    QModelIndex GetCurrentIndex() const;

    virtual int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

private:
    friend class CommandsView;
    CommandsContainerVisual* _commands;
};

class CommandsView : public QListView
{
    Q_OBJECT
public:
    CommandsView(QWidget* p=nullptr);
    void SetModel(CommandsModel* cmd_model);
    CommandsModel* GetModel() const;

private Q_SLOTS:
    void onCurrentIndexChanged(qint32);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection&);

private:
    friend class CommandsModel;
    virtual void setModel(QAbstractItemModel* model) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent* e) Q_DECL_OVERRIDE;

private:
    qint32 _lIndex=-1;
};


#endif // COMMANDSVISUALCOMPONENTS_H
