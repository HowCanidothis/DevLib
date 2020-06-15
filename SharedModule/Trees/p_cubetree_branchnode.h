#ifndef P_CUBETREE_BRANCHNODE_H
#define P_CUBETREE_BRANCHNODE_H

#include <QSet>

#include "cubetree_decl.h"

CUBETREE_BEGIN_NAMESPACE

namespace Private {

template<class T>
class DebugVector : public QVector<T>
{
    using Super = QVector<T>;
public:
    using Super::Super;

    qint32 IterationsCount = 0;
};

template<class ObjectNode>
class BranchNode
{
public:
    //out containers
    typedef DebugVector<ObjectNode*> OutObjectsContainer;
    typedef DebugVector<const BranchNode*> OutBranchesContainer;
    //data containers
    typedef QSet<ObjectNode*> Objects;

    //dependent objects
    struct ObjectsStatistic{
        OutBranchesContainer Branches;
        OutBranchesContainer Leafs;
        OutObjectsContainer BranchObjects;
        OutObjectsContainer LeafObjects;


        void AddBranch(const BranchNode* node)
        {
            Branches.append(node);
            for(ObjectNode *n : node->m_objects) {
                BranchObjects.append(n);
            }
        }
        void AddLeaf(const BranchNode* node)
        {
            Leafs.append(node);
            for(ObjectNode *n : node->m_objects) {
                LeafObjects.append(n);
            }
        }
    };

    struct MemoryStatistic{
        qint32 Branches=0;
        qint32 Leafs=0;
        qint32 BranchObjects=0;
        qint32 LeafObjects=0;
        size_t MemoryUsage=0;

        qint32 TotalObjects() const { return LeafObjects + BranchObjects; }
        qint32 TotalBranches() const { return Branches + Leafs; }
        void AddBranch(const BranchNode* node)
        {
            MemoryUsage += sizeof(BranchNode) + node->m_objects.mem_use();
            Branches++;
            BranchObjects += node->m_objects.size();
        }
        void AddLeaf(const BranchNode* node)
        {
            MemoryUsage += sizeof(BranchNode) + node->m_objects.mem_use();
            Leafs++;
            LeafObjects += node->m_objects.size();
        }
    };

    //constructors
    static BranchNode* CreateLeaf(qint32 maxObjects, const Point& tlf, const Point& brb){ return new BranchNode(maxObjects, BoundingBox(tlf,brb)); }

    //methods
    void Add(ObjectNode *n);
    bool Remove(ObjectNode *n);
    void DestroyBranches();
    void ClearObjects();

    //watchers
    bool IsBranch() const { return !m_leaf; }
    bool IsLeaf() const { return m_leaf; }
    template<typename T> void GetStatistics(T& stats) const;
    void GetBranches(OutBranchesContainer& res) const;
    void GetObjects(OutObjectsContainer& res) const;
    void GetObjectsAt(const BoundingBox& box, OutObjectsContainer& c) const;

    const BoundingBox& GetBoundingBox() const { return m_boundingBox; }

private:
    BranchNode(qint32 maxObjects, const BoundingBox& boundingBox)
        : m_leaf(true)
        , m_maxObjects(maxObjects)
        , m_boundingBox(boundingBox)
    {}
    friend struct MemoryStatistic;
    void split();
    void toBranch() { m_leaf = false; }
    void toLeaf() { m_leaf = true; }

private:
    BranchNode* m_branches[8];
    bool m_leaf;
    qint32 m_maxObjects;
    BoundingBox m_boundingBox;
    Objects m_objects;
};


template<class ObjectNode>
void BranchNode<ObjectNode>::GetBranches(OutBranchesContainer& result) const
{
    result.append(this);
    if(this->IsBranch()){
        for(BranchNode* branch : m_branches) {
            branch->GetBranches(result);
        }
    }
}

template<class ObjectNode>
void BranchNode<ObjectNode>::GetObjectsAt(const BoundingBox& box, OutObjectsContainer& result) const
{
    if(box.Contains(this->m_boundingBox)) {
        this->GetObjects(result);
        return;
    } else if(this->IsBranch()) {
        BoundingBox_Location location = this->m_boundingBox.LocationOfAnother(box);
        result.IterationsCount++;
        switch (location) {
        case Location_Center:
            for(BranchNode* branch : this->m_branches)
                branch->GetObjectsAt(box,result);
            break;
        case Location_CenterBack:
            this->m_branches[Branch_BottomLeftBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopLeftBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightBack]->GetObjectsAt(box, result);
            break;
        case Location_CenterFront:
            this->m_branches[Branch_BottomLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightFront]->GetObjectsAt(box, result);
            break;
        case Location_MiddleBottomBack:
            this->m_branches[Branch_BottomLeftBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightBack]->GetObjectsAt(box, result);
            break;
        case Location_MiddleBottomFront:
            this->m_branches[Branch_BottomLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightFront]->GetObjectsAt(box, result);
            break;
        case Location_MiddleLeftBack:
            this->m_branches[Branch_TopLeftBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomLeftBack]->GetObjectsAt(box, result);
            break;
        case Location_MiddleLeftFront:
            this->m_branches[Branch_TopLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomLeftFront]->GetObjectsAt(box, result);
            break;
        case Location_MiddleRightBack:
            this->m_branches[Branch_TopRightBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightBack]->GetObjectsAt(box, result);
            break;
        case Location_MiddleRightFront:
            this->m_branches[Branch_TopRightFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightFront]->GetObjectsAt(box, result);
            break;
        case Location_MiddleTopBack:
            this->m_branches[Branch_TopLeftBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightBack]->GetObjectsAt(box, result);
            break;
        case Location_MiddleTopFront:
            this->m_branches[Branch_TopLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightFront]->GetObjectsAt(box, result);
            break;
        case Location_TopLeftBack: this->m_branches[Branch_TopLeftBack]->GetObjectsAt(box, result); break;
        case Location_TopRightBack: this->m_branches[Branch_TopRightBack]->GetObjectsAt(box, result); break;
        case Location_BottomLeftBack: this->m_branches[Branch_BottomLeftBack]->GetObjectsAt(box, result); break;
        case Location_BottomRightBack: this->m_branches[Branch_BottomRightBack]->GetObjectsAt(box, result); break;
        case Location_TopLeftFront: this->m_branches[Branch_TopLeftFront]->GetObjectsAt(box, result); break;
        case Location_TopRightFront: this->m_branches[Branch_TopRightFront]->GetObjectsAt(box, result); break;
        case Location_BottomLeftFront: this->m_branches[Branch_BottomLeftFront]->GetObjectsAt(box, result); break;
        case Location_BottomRightFront: this->m_branches[Branch_BottomRightFront]->GetObjectsAt(box, result); break;
        case Location_RightHalf:
            this->m_branches[Branch_BottomRightFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightBack]->GetObjectsAt(box, result);
            break;
        case Location_TopHalf:
            this->m_branches[Branch_TopLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopLeftBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightBack]->GetObjectsAt(box, result);
            break;
        case Location_BottomHalf:
            this->m_branches[Branch_BottomLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomLeftBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightBack]->GetObjectsAt(box, result);
            break;
        case Location_LeftHalf:
            this->m_branches[Branch_TopLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopLeftBack]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomLeftBack]->GetObjectsAt(box, result);
            break;
        case Location_BottomLeftQuart:
            this->m_branches[Branch_BottomLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomLeftBack]->GetObjectsAt(box, result);
            break;
        case Location_TopRightQuart:
            this->m_branches[Branch_TopRightFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopRightBack]->GetObjectsAt(box, result);
            break;
        case Location_BottomRightQuart:
            this->m_branches[Branch_BottomRightFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_BottomRightBack]->GetObjectsAt(box, result);
            break;
        case Location_TopLeftQuart:
            this->m_branches[Branch_TopLeftFront]->GetObjectsAt(box, result);
            this->m_branches[Branch_TopLeftBack]->GetObjectsAt(box, result);
            break;
        default:
            qDebug() << "Unhandled tree part is detected" << location;
            break;
        }
    }

    for(ObjectNode* object : m_objects) {
        result.IterationsCount++;
        if(box.Intersects(object->GetBoundingBox())) {
            result.append(object);
        }
    }
}

template<class ObjectNode>
void BranchNode<ObjectNode>::GetObjects(OutObjectsContainer& result) const
{
    for(auto* object : this->m_objects) {
        result.append(object);
    }
    if(this->IsBranch()) {
        for(BranchNode* branch : m_branches) {
            branch->GetObjects(result);
        }
    }
}

template<class ObjectNode>
void BranchNode<ObjectNode>::Add(ObjectNode* object)
{
    if(this->IsBranch()){
        BoundingBox_Location location = this->m_boundingBox.LocationOfAnother(object->GetBoundingBox());
        switch (location) {
        case Location_BottomLeftBack: this->m_branches[Branch_BottomLeftBack]->Add(object); break;
        case Location_BottomRightBack: this->m_branches[Branch_BottomRightBack]->Add(object); break;
        case Location_TopLeftBack: this->m_branches[Branch_TopLeftBack]->Add(object); break;
        case Location_TopRightBack: this->m_branches[Branch_TopRightBack]->Add(object); break;
        case Location_BottomLeftFront: this->m_branches[Branch_BottomLeftFront]->Add(object); break;
        case Location_BottomRightFront: this->m_branches[Branch_BottomRightFront]->Add(object); break;
        case Location_TopLeftFront: this->m_branches[Branch_TopLeftFront]->Add(object); break;
        case Location_TopRightFront: this->m_branches[Branch_TopRightFront]->Add(object); break;
        default: this->m_objects.insert(object); break;
        }
    }
    else{
        this->m_objects.insert(object);
        if(this->m_objects.size() > m_maxObjects) {
            this->split();
        }
    }
}

template<typename ObjectNode>
void BranchNode<ObjectNode>::split()
{
    Point mid = this->m_boundingBox.GetCenter();
    const auto& tlf = this->m_boundingBox.GetTopLeftFront();
    const auto& brb = this->m_boundingBox.GetBottomRightBack();

    qint32 newMaxObjects = this->m_maxObjects + (this->m_maxObjects >> 2);
    this->m_branches[Branch_TopLeftFront] = this->CreateLeaf(newMaxObjects, tlf, mid);
    this->m_branches[Branch_TopRightFront] = this->CreateLeaf(newMaxObjects, Point(mid.X, tlf.Y, tlf.Z),Point(brb.X, mid.Y, mid.Z));
    this->m_branches[Branch_BottomLeftFront] = this->CreateLeaf(newMaxObjects, Point(tlf.X, mid.Y, tlf.Z),Point(mid.X, brb.Y, mid.Z));
    this->m_branches[Branch_BottomRightFront] = this->CreateLeaf(newMaxObjects, Point(mid.X, mid.Y, tlf.Z), Point(brb.X, brb.Y, mid.Z));
    this->m_branches[Branch_TopLeftBack] = this->CreateLeaf(newMaxObjects, Point(tlf.X, tlf.Y, mid.Z), Point(mid.X, mid.Y, brb.Z));
    this->m_branches[Branch_TopRightBack] = this->CreateLeaf(newMaxObjects, Point(mid.X, tlf.Y, mid.Z),Point(brb.X, mid.Y, brb.Z));
    this->m_branches[Branch_BottomLeftBack] = this->CreateLeaf(newMaxObjects, Point(tlf.X, mid.Y, mid.Z),Point(mid.X, brb.Y, brb.Z));
    this->m_branches[Branch_BottomRightBack] = this->CreateLeaf(newMaxObjects, Point(mid.X, mid.Y, mid.Z), Point(brb.X, brb.Y, brb.Z));

    this->toBranch();
    Objects objects = this->m_objects;
    this->m_objects.clear();
    this->m_objects.reserve(16);
    for(ObjectNode* object : objects){
        BoundingBox_Location location = this->m_boundingBox.LocationOfAnother(object->GetBoundingBox());
        switch (location) {
        case Location_BottomLeftBack: this->m_branches[Branch_BottomLeftBack]->m_objects.insert(object); break;
        case Location_BottomRightBack: this->m_branches[Branch_BottomRightBack]->m_objects.insert(object); break;
        case Location_TopLeftBack: this->m_branches[Branch_TopLeftBack]->m_objects.insert(object); break;
        case Location_TopRightBack: this->m_branches[Branch_TopRightBack]->m_objects.insert(object); break;
        case Location_BottomLeftFront: this->m_branches[Branch_BottomLeftFront]->m_objects.insert(object); break;
        case Location_BottomRightFront: this->m_branches[Branch_BottomRightFront]->m_objects.insert(object); break;
        case Location_TopLeftFront: this->m_branches[Branch_TopLeftFront]->m_objects.insert(object); break;
        case Location_TopRightFront: this->m_branches[Branch_TopRightFront]->m_objects.insert(object); break;
        default: this->m_objects.insert(object); break;
        }
    }
}

template<typename ObjectNode>
bool BranchNode<ObjectNode>::Remove(ObjectNode* object)
{
    if(this->IsBranch()){
        BoundingBox_Location location = this->m_boundingBox.LocationOfAnother(object->GetBoundingBox());
        switch (location) {
        case Location_BottomLeftBack: return this->m_branches[Branch_BottomLeftBack]->Remove(object);
        case Location_BottomRightBack: return this->m_branches[Branch_BottomRightBack]->Remove(object);
        case Location_TopLeftBack: return this->m_branches[Branch_TopLeftBack]->Remove(object);
        case Location_TopRightBack: return this->m_branches[Branch_TopRightBack]->Remove(object);
        case Location_BottomLeftFront: return this->m_branches[Branch_BottomLeftFront]->Remove(object);
        case Location_BottomRightFront: return this->m_branches[Branch_BottomRightFront]->Remove(object);
        case Location_TopLeftFront: return this->m_branches[Branch_TopLeftFront]->Remove(object);
        case Location_TopRightFront: return this->m_branches[Branch_TopRightFront]->Remove(object);
        default: return this->m_objects.remove(object);
        }
    }
    return this->m_objects.remove(object);
}

template<typename ObjectNode>
void BranchNode<ObjectNode>::DestroyBranches()
{
    if(!this->IsLeaf()) {
        for(BranchNode* cbranch : this->m_branches){
            cbranch->DestroyBranches();
            delete cbranch;
        }
        this->toLeaf();
    }
}

template<class ObjectNode>
void BranchNode<ObjectNode>::ClearObjects()
{
    this->m_objects.clear();
}

template<typename ObjectNode> template<typename T>
void BranchNode<ObjectNode>::GetStatistics(T & stats) const
{
    if(this->IsBranch()) {
        stats.AddBranch(this);
        for(BranchNode*cbranch : this->m_branches) {
            cbranch->GetStatistics(stats);
        }
    } else {
        stats.AddLeaf(this);
    }
}

}
CUBETREE_END_NAMESPACE

#endif // P_QUADTREEBRANCHNODE_H
