#ifndef COMPUTEGRAPHCORE_H
#define COMPUTEGRAPHCORE_H

#include "computegraphbase.h"

class InputFrameStream;

#define ADD_PROPERTY_NODEA(NodeClass,PropertyName,AdditionalName) \
    private: \
    class NodeClass* PropertyName; \
    public: \
    class NodeClass* Get##NodeClass##AdditionalName() { return PropertyName; }
#define ADD_PROPERTY_NODE(NodeClass,PropertyName) ADD_PROPERTY_NODEA(NodeClass,PropertyName,)

class ComputeGraphCore : public GtComputeGraph
{
public:
    static ComputeGraphCore* Instance() { static ComputeGraphCore res; return &res; }

    void Initialize(InputFrameStream* stream);

    GtComputeNodeBase* GetRootNode() const;

    const Stack<GtComputeNodeBase*>& GetNodes() const { return _computeNodes; }

private:
    template<class T, typename ... Args> void createNode(T*& ptr, Args ... args) {
        ptr = new T(args...);
        _computeNodes.Push(ptr);
    }

private:
    ComputeGraphCore();
    ADD_PROPERTY_NODE(ComputeNodeBlackHole, _blackHole)
    ADD_PROPERTY_NODE(ComputeNodeCrop, _crop)
    ADD_PROPERTY_NODE(ComputeNodeDelay, _delay)
    ADD_PROPERTY_NODE(ComputeNodeDepthFakeSensor, _sensor)
    ADD_PROPERTY_NODE(ComputeNodeHolesFilter, _holesFilter)
    ADD_PROPERTY_NODE(ComputeNodeResize, _resize)
    ADD_PROPERTY_NODE(ComputeNodeMinResize, _minResize)
    ADD_PROPERTY_NODE(ComputeNodeVolcanoRecognition, _volcano)

    ADD_PROPERTY_NODEA(ComputeNodeBlur, _postBlur, Post)

    ADD_PROPERTY_NODE(ComputeNodeDepthSensor, _depthSensor)

    Stack<GtComputeNodeBase*> _computeNodes;
};

#endif // COMPUTEGRAPHCORE_H
