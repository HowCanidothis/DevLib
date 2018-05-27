#ifndef COMPUTEGRAPHCORE_H
#define COMPUTEGRAPHCORE_H

#include "computegraphbase.h"

class InputFrameStream;

#define ADD_PROPERTY_NODEA(NodeClass,PropertyName,AdditionalName) \
    private: \
    class NodeClass* PropertyName; \
    public: \
    class NodeClass* get##NodeClass##AdditionalName() { return PropertyName; }
#define ADD_PROPERTY_NODE(NodeClass,PropertyName) ADD_PROPERTY_NODEA(NodeClass,PropertyName,)

class ComputeGraphCore : public GtComputeGraph
{
public:
    static ComputeGraphCore* instance() { static ComputeGraphCore res; return &res; }

    void initialize(InputFrameStream* stream);

    GtComputeNodeBase* getRootNode() const;

    const Stack<GtComputeNodeBase*>& getNodes() const { return compute_nodes; }

private:
    template<class T, typename ... Args> void createNode(T*& ptr, Args ... args) {
        ptr = new T(args...);
        compute_nodes.Push(ptr);
    }

private:
    ComputeGraphCore();
    ADD_PROPERTY_NODE(ComputeNodeBlackHole, black_hole)
    ADD_PROPERTY_NODE(ComputeNodeCrop, crop)
    ADD_PROPERTY_NODE(ComputeNodeDelay, delay)
    ADD_PROPERTY_NODE(ComputeNodeDepthFakeSensor, sensor)
    ADD_PROPERTY_NODE(ComputeNodeHolesFilter, holes_filter)
    ADD_PROPERTY_NODE(ComputeNodeResize, resize)
    ADD_PROPERTY_NODE(ComputeNodeMinResize, min_resize)
    ADD_PROPERTY_NODE(ComputeNodeVolcanoRecognition, volcano)

    ADD_PROPERTY_NODEA(ComputeNodeBlur, post_blur, Post)

    ADD_PROPERTY_NODE(ComputeNodeDepthSensor, depth_sensor)

    Stack<GtComputeNodeBase*> compute_nodes;
};

#endif // COMPUTEGRAPHCORE_H
