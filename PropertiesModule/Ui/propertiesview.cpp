#include "propertiesview.h"

#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QProcess>
#include <QAction>
#include <QMouseEvent>

#include <PropertiesModule/internal.hpp>
#include <WidgetsModule/internal.hpp>

#include "propertiesmodel.h"
#include "SharedGuiModule/decl.h"
#include "propertiesdelegate.h"



PropertiesView::PropertiesView(QWidget* parent, Qt::WindowFlags flags)
    : PropertiesView(PropertiesSystem::GetCurrentScope()->GetName(), parent, flags)
{

}

PropertiesView::PropertiesView(const PropertiesScopeName& scope, QWidget* parent, Qt::WindowFlags flags)
    : Super(parent)
    , m_defaultTextEditor("Common/TextEditor", PropertiesSystem::Global)
{
    setWindowFlags(windowFlags() | flags);
    setItemDelegate(new PropertiesDelegate(this));
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    header()->hide();
    setIndentation(10);
    setAnimated(true);

    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    m_propertiesModel = new PropertiesModel(scope, this);
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(m_propertiesModel);
    setModel(proxy);

    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    m_actionOpenWithTextEditor = MenuWrapper(this).AddAction(tr("Open with text editor"), [this](){
        QString openFile = m_indexUnderCursor.data().toString();

        QStringList arguments { openFile };

        QProcess *process = new QProcess(this);
        connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
        process->start(m_defaultTextEditor, arguments);

        qCWarning(LC_SYSTEM) << "Opening" << m_defaultTextEditor << arguments;
    });

    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void PropertiesView::Update(const FAction& action)
{
    m_propertiesModel->Change(action);
}

void PropertiesView::SetContextIndex(const Name& contextIndex)
{
    m_propertiesModel->Scope = contextIndex;
}

const Name& PropertiesView::GetContextIndex() const
{
    return m_propertiesModel->Scope;
}

void PropertiesView::Save(const QString& fileName)
{
    m_propertiesModel->Save(fileName);
}

void PropertiesView::Load(const QString& fileName)
{
    m_propertiesModel->Load(fileName);
}

void PropertiesView::showEvent(QShowEvent*)
{
    if(!model()->rowCount()) {
        m_propertiesModel->Change([]{});
    }
}

void PropertiesView::mouseReleaseEvent(QMouseEvent* event)
{
    if(event->button() == Qt::RightButton) {
        m_indexUnderCursor = this->indexAt(event->pos());
        validateActionsVisiblity();
    }
    QAbstractItemView::State preState = state();
    QTreeView::mouseReleaseEvent(event);
    if (preState == QAbstractItemView::AnimatingState)
        setState(preState);
}

void PropertiesView::validateActionsVisiblity()
{
    if(m_defaultTextEditor.IsValid() && m_indexUnderCursor.data(Property::RoleDelegateValue).toInt() == Property::DelegateFileName) {
        m_actionOpenWithTextEditor->setVisible(true);
    }
    else {
        m_actionOpenWithTextEditor->setVisible(false);
    }
}
