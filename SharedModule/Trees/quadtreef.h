#ifndef QUADTREETREEF_H
#define QUADTREETREEF_H

#include "p_quadtreef_branchnode.h"

class QImage;

#if !defined (QT_GUI_LIB) && !defined(QUADTREE_NO_IMAGE_OUTPUT)
#define QUADTREE_NO_IMAGE_OUTPUT
#endif

#ifndef QUADTREE_NO_IMAGE_OUTPUT
#include <QPainter>
#include <QImage>
#include <QTextStream>
#endif

QUADTREE_BEGIN_NAMESPACE

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF=60>
class Tree
{
    typedef typename Private::BranchNode<ObjectNode> BranchNode;
private:
    BranchNode *root;
    Tree(const Tree& other) = delete;
    Tree& operator =(const Tree& other) const = delete;
public:
    typedef typename BranchNode::OutObjectsContainer OutObjectsContainer;
    typedef typename BranchNode::OutBranchesContainer OutBranchesContainer;
    Tree(real x1, real y1, real width, real height);
    ~Tree();

    //methods
    void Clear();
    void Compress(){ root->compress(); }
    ObjectNode* AddObject(ObjectNode* node){ root->add(node); return node; }
    bool RemoveObject(const ObjectNode* node)
    {
        if(root->remove(const_cast<ObjectNode*>(node))) {
            OnObjectRemoved(const_cast<ObjectNode*>(node));
            return true;
        } else {
            return false;
        }
    }
    void UpdateObject(const ObjectNode* node){ if(root->remove(node)) root->add(node); }
    void Reconstruct();

    //watchers
#ifndef QUADTREE_NO_IMAGE_OUTPUT
    enum DrawMask{
        DrawLeafObjects = 0x1,
        DrawLeafObjectsObjects = 0x2,
        DrawBranchObjectsObjects = 0x4,
        DrawBranchesWithObjects = 0x8,
        DrawSelectionRect=0x10,

        DrawBranches = DrawBranchesWithObjects | DrawBranchObjectsObjects,
        DrawLeafs = DrawLeafObjectsObjects | DrawLeafObjects,
        DrawAll = DrawBranches | DrawLeafs | DrawSelectionRect
    };

    void ToImage(QImage* img, BoundingRect* rect=0, qint32 dm= DrawAll) const;
    void Print(QTextStream* stream) const;
#endif
    void GetObjectsAt(const BoundingRect& rect, OutObjectsContainer& c) const { root->getObjectsAt(rect,c); }
    OutObjectsContainer GetObjectsAt(const BoundingRect& rect) const{ OutObjectsContainer res; root->getObjectsAt(rect,res); return res; }
    OutObjectsContainer GetObjectsAtClosestTo(const BoundingRect& rect) const;
    OutObjectsContainer GetObjectsAll() const { OutObjectsContainer res; root->getObjects(res); return res; }

    CommonDispatcher<ObjectNode*> OnObjectRemoved;
};

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::Tree(real x1, real y1, real width, real height)
{
    root = BranchNode::createLeaf(MAX_OBJECTS_PER_LEAF, Point(x1, y1), Point(x1 + width, y1 + height));
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::~Tree()
{
    Clear();
    delete root;
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
inline void Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::Clear()
{
    root->objects.clear();
    root->destroyBranches();
    root->toLeaf();
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
void Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::Reconstruct() {
    OutObjectsContainer objects = this->getObjectsAll();
    this->clear();
    for(ObjectNode *o : objects)
        root->add(o);
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
typename Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::OutObjectsContainer Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::GetObjectsAtClosestTo(const BoundingRect& rect) const
{
    OutObjectsContainer res;
    root->getObjectsAt(rect, res);
    auto center = rect.getCenter();
    auto x = center.x, y = center.y;
    std::sort(res.begin(), res.end(), [x,y](const ObjectNode* f, const ObjectNode* s) { return f->getBoundingRect().getCenter().squaredDistanceToPoint(x,y) < s->getBoundingRect().getCenter().squaredDistanceToPoint(x,y); });
    return res;
}

#ifndef QUADTREE_NO_IMAGE_OUTPUT
template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
void Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::ToImage(QImage* img, BoundingRect* rect, qint32 dm) const
{
    img->fill(Qt::black);
    struct Scaler{
        real Xscale;
        real Yscale;
        Scaler(real X, real Y) : Xscale(X), Yscale(Y) {}
        QRect scale(const BoundingRect& rect){
            Point tl = rect.getTopLeft();
            Point br = rect.getBottomRight();
            QRect res(QPoint(tl.x * Xscale, tl.y * Yscale),QPoint(br.x * Xscale, br.y * Yscale));
            return res;
        }
    };

    real scaleX = real(img->width()) / root->br.getWidth();
    real scaleY = real(img->height()) / root->br.getHeight();
    Scaler scaler(scaleX,scaleY);
    QPainter pntr(img);
    typename BranchNode::ObjectsStatistic stats;
    root->getStatistics(stats);

    if(dm & DrawLeafObjects){
        pntr.setPen(QPen(Qt::red, 1));
        for(const BranchNode *br : stats.leafs)
            pntr.drawRect(scaler.scale(br->br));
    }
    if(dm & DrawBranchesWithObjects)
    {
        pntr.setPen(QPen(Qt::white, 1));
        for(const BranchNode *br : stats.branches)
            pntr.drawRect(scaler.scale(br->br));
    }

    if(rect){
        Point center = rect->getCenter();
        OutObjectsContainer objects = this->getObjectsAtClosestTo(*rect, center.x, center.y);
        QColor col(Qt::blue);
        float step = float(255) / objects.size();
        float cAlpha = 255;
        for(const ObjectNode *n : objects){
            pntr.setPen(col);
            pntr.drawRect(scaler.scale(n->getBoundingRect()));
            cAlpha -= step;
            col.setAlpha(cAlpha);
        }

        if(dm & DrawSelectionRect){
            pntr.setPen(QPen(Qt::green, 1));
            pntr.drawRect(scaler.scale(*rect));
        }
    }
    else{
        if(dm & DrawLeafObjectsObjects){
            pntr.setPen(QPen(Qt::blue, 1));
            for(const ObjectNode * n : stats.leaf_objects)
                pntr.drawRect(scaler.scale(n->getBoundingRect()));
        }
        if(dm & DrawBranchObjectsObjects)
        {
            pntr.setPen(QPen(Qt::yellow, 1));
            for(const ObjectNode * n : stats.branch_objects)
                pntr.drawRect(scaler.scale(n->getBoundingRect()));
        }

    }
}

template<typename ObjectNode, qint32 MAX_OBJECTS_PER_LEAF>
void Tree<ObjectNode,MAX_OBJECTS_PER_LEAF>::Print(QTextStream* stream) const
{
    typename BranchNode::MemoryStatistic stat;
    root->getStatistics(stat);

    stat.mem_usage += sizeof(*this);
    *stream << "==============================QuadTree mem_report=========================\n";
    *stream << "total branches and leafs: " << (stat.totalBranches()) << "\n";
    *stream << "leafs: " << stat.leafs << "\n";
    *stream << "branches: " << stat.branches << "\n";
    *stream << "large objects: " << stat.branch_objects << "\n";
    *stream << "small objects: " << stat.leaf_objects << "\n";
    *stream << "memory usage: " << stat.mem_usage << "\n";
    *stream << "total memory usage: " << (stat.mem_usage + sizeof(ObjectNode) * stat.totalObjects()) << "\n";
    *stream << "==========================================================================\n";
}

#endif //QUADTREE_NO_IMAGE_OUTPUT

QUADTREE_END_NAMESPACE

#endif // QUADTREE_H
