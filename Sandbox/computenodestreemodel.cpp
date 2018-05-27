#include "computenodestreemodel.h"

#include "ComputeGraphModule/computenodebase.h"
#include "ComputeGraphModule/computegraphbase.h"

GtComputeNodesTreeModel::TreeNode* GtComputeNodesTreeModel::asNode(const QModelIndex& index) const {
    return (TreeNode*)index.internalPointer();
}

GtComputeNodesTreeModel::GtComputeNodesTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , compute_graph(nullptr)
{

}

void GtComputeNodesTreeModel::setRootNode(GtComputeNodeBase* node, GtComputeGraph* compute_graph)
{
    layoutAboutToBeChanged();
    if(!tree.isEmpty()) {
        for(TreeNode* node : adapters::range(tree.keyBegin(), tree.keyEnd())) {
            delete node;
        }
        tree.clear();
    }

    this->compute_graph = compute_graph;
    TreeNode* root = new TreeNode(nullptr, node);
    tree[nullptr].Append(root);
    addParentNode(root);
    layoutChanged();
}

void GtComputeNodesTreeModel::update()
{
    auto find = tree.find(nullptr);
    if(find != tree.end()) {
        const Array<TreeNode*>& ar = find.value();
        if(!ar.IsEmpty()) {
            setRootNode(ar.First()->node, compute_graph);
        }
    }
}


QModelIndex GtComputeNodesTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column, getChilds(asNode(parent)).At(row)) : QModelIndex();
}

QModelIndex GtComputeNodesTreeModel::parent(const QModelIndex& child) const
{
    TreeNode* node = asNode(child);
    if(node->parent == nullptr)
        return QModelIndex();
    const Array<TreeNode*>& childs = getChilds(node->parent->parent);
    auto find = childs.FindSorted(node->parent);
    return createIndex(std::distance(childs.Begin(), find), 0, node->parent);
}

int GtComputeNodesTreeModel::rowCount(const QModelIndex& parent) const
{
    if(tree.isEmpty()) return 0;
    qint32 result = getChilds(asNode(parent)).Size();
    return result;
}

int GtComputeNodesTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant GtComputeNodesTreeModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) return QVariant();

    switch (role) {
    case Qt::DisplayRole: return asNode(index)->node->GetName();
    case Qt::CheckStateRole: return asNode(index)->node->IsEnabled() ? Qt::Checked : Qt::Unchecked;
    default:
        break;
    }

    return QVariant();
}

void GtComputeNodesTreeModel::addParentNode(TreeNode* parent_node)
{
    auto& childs = getChilds(parent_node);
    for(GtComputeNodeBase* child_node : parent_node->node->_linkedOutputs) {
        TreeNode* tree_node = new TreeNode(parent_node, child_node);
        childs.InsertSortedUnique(tree_node);
        addParentNode(tree_node);
    }
}


Qt::ItemFlags GtComputeNodesTreeModel::flags(const QModelIndex&) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

bool GtComputeNodesTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid() || role != Qt::CheckStateRole)
        return false;

    GtComputeNodeBase* node = asNode(index)->node;
    bool enabled = value.toBool();

    if(compute_graph) {
        compute_graph->Asynch([enabled, node]() { node->SetEnabled(enabled); });
    }
    else {
        node->SetEnabled(enabled);
    }

    return true;
}
