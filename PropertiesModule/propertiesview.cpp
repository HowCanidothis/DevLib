#include "propertiesview.h"

#ifdef QT_GUI_LIB

#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QProcess>
#include <QAction>
#include <QMouseEvent>

#include <SharedModule/External/utils.h>

#include "PropertiesModule/propertiessystem.h"
#include "propertiesmodel.h"
#include "SharedGuiModule/decl.h"
#include "Widgets/propertiesdelegate.h"


PropertiesView::PropertiesView(QWidget* parent, Qt::WindowFlags flags)
    : PropertiesView(PropertiesSystem::Global, parent, flags)
{

}

PropertiesView::PropertiesView(qint32 contextIndex, QWidget* parent, Qt::WindowFlags flags)
    : Super(parent)
    , m_defaultTextEditor("Common/TextEditor", PropertiesSystem::Global)
{
    setWindowFlags(windowFlags() | flags);
    setItemDelegate(new PropertiesDelegate(this));
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    header()->hide();
    setIndentation(0);
    setAnimated(true);

    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);

    m_propertiesModel = new PropertiesModel(contextIndex, this);
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(m_propertiesModel);
    setModel(proxy);

    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    m_actionOpenWithTextEditor = createAction(tr("Open with text editor"), [this](){
        QString openFile = m_indexUnderCursor.data().toString();

        QStringList arguments { openFile };

        QProcess *process = new QProcess(this);
        connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
        process->start(m_defaultTextEditor, arguments);

        qCWarning(LC_SYSTEM) << "Opening" << m_defaultTextEditor << arguments;
    });
    addAction(m_actionOpenWithTextEditor);

    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void PropertiesView::Update(const FAction& action)
{
    m_propertiesModel->Change(action);
}

void PropertiesView::SetContextIndex(qint32 contextIndex)
{
    m_propertiesModel->ContextIndex = contextIndex;
}

qint32 PropertiesView::GetContextIndex() const
{
    return m_propertiesModel->ContextIndex;
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

#endif
