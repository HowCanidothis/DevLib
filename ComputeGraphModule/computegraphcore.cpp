#include "computegraphcore.h"

#include "computenodedepthfakesensor.h"
#include "computenodeblur.h"
#include "computenodeholesfilter.h"
#include "computenodevolcanorecognition.h"
#include "computenoderesize.h"
#include "computenodecrop.h"
#include "computenodeblackhole.h"
#include "computenodedelay.h"
#include "computenodegaussenblur.h"
#include "computenodemedianblur.h"
#include "computenodeminresize.h"

#include "computenodedepthsensor.h"

#include "PropertiesModule/propertiessystem.h"

ComputeGraphCore::ComputeGraphCore()
    : GtComputeGraph(30)
{
    PropertiesSystem::Begin() = [this](const auto& setter) {
        this->asynch(setter);
    };

    createNode<ComputeNodeBlackHole>(black_hole, "compute/ComputeNodeBlackHole");
    createNode<ComputeNodeCrop>(crop, "compute/ComputeNodeCrop");
    createNode<ComputeNodeDelay>(delay, "compute/ComputeNodeDelay");
    createNode<ComputeNodeDepthFakeSensor>(sensor, "compute/ComputeNodeDepthFakeSensor");
    createNode<ComputeNodeHolesFilter>(holes_filter, "compute/ComputeNodeHolesFilter");
    createNode<ComputeNodeResize>(resize, "compute/ComputeNodeResize");
    createNode<ComputeNodeVolcanoRecognition>(volcano, "compute/ComputeNodeVolcanoRecognition");
    createNode<ComputeNodeBlur>(post_blur, "compute/ComputeNodeBlur");
    createNode<ComputeNodeMinResize>(min_resize, "compute/ComputeNodeMinResize");

    createNode<ComputeNodeDepthSensor>(depth_sensor, "compute/ComputeNodeDepthSensor");

    PropertiesSystem::End();
}

void ComputeGraphCore::initialize(InputFrameStream* stream)
{
    sensor->setInputStream(stream);

    crop->setInput(getRootNode());
    holes_filter->setInput(crop);

    // branch 1
    min_resize->setInput(holes_filter);
    black_hole->setInput(min_resize);

    // branch 2
    delay->setInput(holes_filter);
    delay->setMotionMask(black_hole->getOutput());

    volcano->setInput(delay);

    post_blur->setInput(delay);

    addCalculationGraph(getRootNode());
}

GtComputeNodeBase* ComputeGraphCore::getRootNode() const
{
    return sensor;
}
