#ifndef P_QUADTREE_BRANCHNODE_H
#define P_QUADTREE_BRANCHNODE_H
#include "quadtreef_decl.h"
#include <QSet>
QUADTREE_BEGIN_NAMESPACE

namespace Private {

template<class ObjectNode>
class BranchNode
{
public:
    //out containers
    typedef QVector<const ObjectNode*> OutObjectsContainer;
    typedef QVector<const BranchNode*> OutBranchesContainer;
    //data containers
    typedef QSet<ObjectNode*> Objects;

    //dependent objects
    struct ObjectsStatistic{
        OutBranchesContainer branches;
        OutBranchesContainer leafs;
        OutObjectsContainer branch_objects;
        OutObjectsContainer leaf_objects;


        void addBranch(const BranchNode *br){ branches.append(br); for(ObjectNode *n : br->objects) branch_objects.append(n); }
        void addLeaf(const BranchNode *br){ leafs.append(br); for(ObjectNode *n : br->objects) leaf_objects.append(n); }
    };

    struct MemoryStatistic{
        qint32 branches=0;
        qint32 leafs=0;
        qint32 branch_objects=0;
        qint32 leaf_objects=0;
        size_t mem_usage=0;

        qint32 totalObjects() const { return leaf_objects + branch_objects; }
        qint32 totalBranches() const { return branches + leafs; }
        void addBranch(const BranchNode *br){ mem_usage += sizeof(BranchNode) + br->objects.mem_use(); branches++; branch_objects += br->objects.size(); }
        void addLeaf(const BranchNode *br){ mem_usage += sizeof(BranchNode) + br->objects.mem_use(); leafs++; leaf_objects += br->objects.size(); }
    };

    //constructors
    static BranchNode *createLeaf(qint32 max_objects, const Point &tl, const Point& br){ return new BranchNode(max_objects, BoundingRect(tl,br)); }

    //methods
    void compress();
    void add(ObjectNode *n);
    bool remove(ObjectNode *n);
    void destroyBranches();

    void toBranch() { leaf = false; }
    void toLeaf() { leaf = true; }

    //watchers
    bool isBranch() const { return !leaf; }
    bool isLeaf() const { return leaf; }
    template<typename T> void getStatistics(T &stats) const;
    void getBranches(OutBranchesContainer &res) const;
    void getObjects(OutObjectsContainer &res) const;
    void getObjectsAt(const BoundingRect &rect, OutObjectsContainer &c) const;

    //vars
private:
    BranchNode(qint32 max_count_per_leaf, const BoundingRect& _br) : max_objects(max_count_per_leaf), br(_br) { }
    //private vars
    friend struct MemoryStatistic;
    void split();
    BranchNode *branches[4];
    bool leaf=true;
public:
    //public vars
    qint32 max_objects;
    BoundingRect br;
    Objects objects;
};


template<class ObjectNode>
void BranchNode<ObjectNode>::getBranches(OutBranchesContainer &res) const
{
    res.append(this);
    if(this->isBranch()){
        for(BranchNode *br : branches)
            br->getBranches(res);
    }
}

template<class ObjectNode>
void BranchNode<ObjectNode>::getObjectsAt(const BoundingRect &rect, OutObjectsContainer &c) const
{
    if(rect.contains(this->br))
        this->getObjects(c);
    else if(this->isBranch()){
        BoundingRect_Location l = this->br.locationOfOther(rect);
        switch (l) { //this is all done only for testing largeNodes only if it is realy nessesary(small optimization speed if largeNodes are few)
        case Location_Center:
            for(BranchNode *br : this->branches)
                br->getObjectsAt(rect,c);
            break;
        case Location_MiddleBottom:
            this->branches[Branch_BottomLeft]->getObjectsAt(rect,c);
            this->branches[Branch_BottomRight]->getObjectsAt(rect,c);
            break;
        case Location_MiddleLeft:
            this->branches[Branch_TopLeft]->getObjectsAt(rect,c);
            this->branches[Branch_BottomLeft]->getObjectsAt(rect,c);
            break;
        case Location_MiddleRight:
            this->branches[Branch_TopRight]->getObjectsAt(rect,c);
            this->branches[Branch_BottomRight]->getObjectsAt(rect,c);
            break;
        case Location_MiddleTop:
            this->branches[Branch_TopLeft]->getObjectsAt(rect,c);
            this->branches[Branch_TopRight]->getObjectsAt(rect,c);
            break;
        case Location_TopLeft: this->branches[Branch_TopLeft]->getObjectsAt(rect,c); break;
        case Location_TopRight: this->branches[Branch_TopRight]->getObjectsAt(rect,c); break;
        case Location_BottomLeft: this->branches[Branch_BottomLeft]->getObjectsAt(rect,c); break;
        case Location_BottomRight: this->branches[Branch_BottomRight]->getObjectsAt(rect,c); break;
        default:
            break;
        }
        for(ObjectNode *n : objects)
            if(rect.intersects(n->getBoundingRect())) c.append(n);
    }
    else{ //Leaf
        for(ObjectNode *n : objects)
            if(rect.intersects(n->getBoundingRect())) c.append(n);
    }
}

template<class ObjectNode>
void BranchNode<ObjectNode>::getObjects(OutObjectsContainer &res) const
{
    res.reserve(res.size() + objects.size());
    for(auto* object : this->objects) {
        res.append(object);
    }
    if(this->isBranch()){
        for(BranchNode *br : branches)
            br->getObjects(res);
    }
}


template<class ObjectNode>
void BranchNode<ObjectNode>::compress()
{
    if(this->isBranch()){
        for(BranchNode*cbranch : this->branches)
            cbranch->compress();
    }
    this->objects.compress();
}

template<class ObjectNode>
void BranchNode<ObjectNode>::add(ObjectNode *n)
{
    if(this->isBranch()){
        BoundingRect_Location l = this->br.locationOfOther(n->getBoundingRect());
        switch (l) {
        case Location_BottomLeft: this->branches[Branch_BottomLeft]->add(n); break;
        case Location_BottomRight: this->branches[Branch_BottomRight]->add(n); break;
        case Location_TopLeft: this->branches[Branch_TopLeft]->add(n); break;
        case Location_TopRight: this->branches[Branch_TopRight]->add(n); break;
        default: this->objects.insert(n); break;
        }
    }
    else{
        this->objects.insert(n);
        if(this->objects.size() > max_objects)
            this->split();
    }
}

template<typename ObjectNode>
void BranchNode<ObjectNode>::split()
{
    Point mid = this->br.getCenter();
    Point tl = this->br.getTopLeft();
    Point br = this->br.getBottomRight();

    qint32 nmax_objects = this->max_objects + (this->max_objects >> 2);
    this->branches[Branch_TopLeft] = this->createLeaf(nmax_objects, tl,mid);
    this->branches[Branch_TopRight] = this->createLeaf(nmax_objects, Point(mid.x, tl.y),Point(br.x, mid.y));
    this->branches[Branch_BottomLeft] = this->createLeaf(nmax_objects, Point(tl.x, mid.y),Point(mid.x, br.y));
    this->branches[Branch_BottomRight] = this->createLeaf(nmax_objects, mid,br);

    this->toBranch();
    Objects c = this->objects;
    this->objects.clear();
    this->objects.reserve(16);
    for(ObjectNode *n : c){
        BoundingRect_Location l = this->br.locationOfOther(n->getBoundingRect());
        switch (l) {
        case Location_BottomLeft: this->branches[Branch_BottomLeft]->objects.insert(n); break;
        case Location_BottomRight: this->branches[Branch_BottomRight]->objects.insert(n); break;
        case Location_TopLeft: this->branches[Branch_TopLeft]->objects.insert(n); break;
        case Location_TopRight: this->branches[Branch_TopRight]->objects.insert(n); break;
        default: this->objects.insert(n); break;
        }
    }
}

template<typename ObjectNode>
bool BranchNode<ObjectNode>::remove(ObjectNode *n)
{
    if(this->isBranch()){
        BoundingRect_Location l = this->br.locationOfOther(n->getBoundingRect());
        switch (l) {
        case Location_BottomLeft: return this->branches[Branch_BottomLeft]->remove(n);
        case Location_BottomRight: return this->branches[Branch_BottomRight]->remove(n);
        case Location_TopLeft: return this->branches[Branch_TopLeft]->remove(n);
        case Location_TopRight: return this->branches[Branch_TopRight]->remove(n);
        default: return this->objects.remove(n);
        }
    }
    return this->objects.remove(n);
}

template<typename ObjectNode>
void BranchNode<ObjectNode>::destroyBranches()
{
    if(!this->isLeaf()){
        for(BranchNode*cbranch : this->branches){
            cbranch->destroyBranches();
            delete cbranch;
        }
    }
}

template<typename ObjectNode> template<typename T>
void BranchNode<ObjectNode>::getStatistics(T & stats) const
{
    if(this->isBranch()){
        stats.addBranch(this);
        for(BranchNode*cbranch : this->branches)
            cbranch->getStatistics(stats);
    }
    else{
        stats.addLeaf(this);
    }
}

}
QUADTREE_END_NAMESPACE

#endif // P_QUADTREEBRANCHNODE_H
