#ifndef GTCOMPUTENODESTREEMODEL_H
#define GTCOMPUTENODESTREEMODEL_H

#include <QAbstractItemModel>

#include "SharedModule/array.h"

class GtComputeNodeBase;
class GtComputeGraph;

class GtComputeNodesTreeModel : public QAbstractItemModel
{
    struct TreeNode {
        TreeNode* parent;
        GtComputeNodeBase* node;

        TreeNode(TreeNode* parent, GtComputeNodeBase* node)
            : parent(parent)
            , node(node)
        {}
    };
public:
    GtComputeNodesTreeModel(QObject* parent=0);

    void setRootNode(GtComputeNodeBase* node, GtComputeGraph* compute_graph);
    void update();
    // QAbstractItemModel interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex& child) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
private:
    void addParentNode(TreeNode* parent_node);
    TreeNode* asNode(const QModelIndex& index) const;
    const Array<TreeNode*>& getChilds(TreeNode* parent) const { return tree.find(parent).value(); }
    Array<TreeNode*>& getChilds(TreeNode* parent) { return tree[parent]; }
    void reset();
private:
//    template<class T> using Array = QVector<T>;

    QHash<TreeNode*, Array<TreeNode*>> tree;
    GtComputeGraph* compute_graph;
};

#endif // GTCOMPUTENODESTREEMODEL_H
