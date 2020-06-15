#ifndef CUBETREETREE_H
#define CUBETREETREE_H

#include <QTextStream>

#include "SharedModule/dispatcher.h"
#include "p_cubetree_branchnode.h"

CUBETREE_BEGIN_NAMESPACE

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF=60>
class Tree
{
    typedef typename Private::BranchNode<ObjectNode> BranchNode;
private:
    BranchNode* m_root;
    Tree(const Tree& other) = delete;
    Tree& operator =(const Tree& other) const = delete;
public:
    typedef typename BranchNode::OutObjectsContainer OutObjectsContainer;
    typedef typename BranchNode::OutBranchesContainer OutBranchesContainer;
    Tree(real centerX, real centerY, real centerZ, real width, real height, real depth);
    ~Tree();

    //methods
    void Clear();
    void AddObject(ObjectNode* object){ m_root->Add(object); }
    bool RemoveObject(ObjectNode* object);
    void UpdateObject(const ObjectNode* object){ if(m_root->Remove(object)) m_root->Add(object); }
    void Reconstruct();

    void Print(QTextStream& stream) const;
    void GetObjectsAt(const BoundingBox& box, OutObjectsContainer& result) const { m_root->GetObjectsAt(box, result); }
    OutObjectsContainer GetObjectsAt(const BoundingBox& box) const{ OutObjectsContainer res; this->GetObjectsAt(box,res); return res; }
    OutObjectsContainer GetObjectsAtClosestToCenter(const BoundingBox& box) const;
    OutObjectsContainer GetObjectsAll() const { OutObjectsContainer res; m_root->GetObjects(res); return res; }
    OutBranchesContainer GetBranchesAll() const { OutBranchesContainer res; m_root->GetBranches(res); return res; }

    CommonDispatcher<ObjectNode*> OnObjectRemoved;
};

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::Tree(real centerX, real centerY, real centerZ, real width, real height, real depth)
{
    m_root = BranchNode::CreateLeaf(MAX_OBJECTS_PER_LEAF, Point(centerX - width / 2, centerY + height / 2, centerZ + depth / 2), Point(centerX + width / 2, centerY - height / 2, centerZ - depth / 2));
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::~Tree()
{
    Clear();
    delete m_root;
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
inline void Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::Clear()
{
    m_root->ClearObjects();
    m_root->DestroyBranches();
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
bool Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::RemoveObject(ObjectNode* object)
{
    if(m_root->Remove(const_cast<ObjectNode*>(object))) {
        OnObjectRemoved(object);
        return true;
    }
    return false;
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
void Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::Reconstruct() {
    OutObjectsContainer objects = GetObjectsAll();
    this->Clear();
    for(ObjectNode *o : objects)
        m_root->Add(o);
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
typename Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::OutObjectsContainer Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::GetObjectsAtClosestToCenter(const BoundingBox& box) const
{
    OutObjectsContainer res;
    m_root->GetObjectsAt(box, res);
    auto center = box.GetCenter();
    std::sort(res.begin(), res.end(), [&center](const ObjectNode *f, const ObjectNode *s) { return f->GetBoundingBox().GetCenter().SquaredDistanceToPoint(center) < s->GetBoundingBox().GetCenter().SquaredDistanceToPoint(center); });
    return res;
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
void Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::Print(QTextStream& stream) const
{
    typename BranchNode::MemoryStatistic stat;
    m_root->getStatistics(stat);

    stat.MemoryUsage += sizeof(*this);
    stream << "==============================CubeTree mem_report=========================\n";
    stream << "total branches and leafs: " << (stat.TotalBranches()) << "\n";
    stream << "leafs: " << stat.Leafs << "\n";
    stream << "branches: " << stat.Branches << "\n";
    stream << "large objects: " << stat.BranchObjects << "\n";
    stream << "small objects: " << stat.LeafObjects << "\n";
    stream << "memory usage: " << stat.MemoryUsage << "\n";
    stream << "total memory usage: " << (stat.MemoryUsage + sizeof(ObjectNode) * stat.TotalObjects()) << "\n";
    stream << "==========================================================================\n";
}

CUBETREE_END_NAMESPACE

#endif // QUADTREE_H
