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
        this->Asynch(setter);
    };

    createNode<ComputeNodeBlackHole>(_blackHole, "compute/ComputeNodeBlackHole");
    createNode<ComputeNodeCrop>(_crop, "compute/ComputeNodeCrop");
    createNode<ComputeNodeDelay>(_delay, "compute/ComputeNodeDelay");
    createNode<ComputeNodeDepthFakeSensor>(_sensor, "compute/ComputeNodeDepthFakeSensor");
    createNode<ComputeNodeHolesFilter>(_holesFilter, "compute/ComputeNodeHolesFilter");
    createNode<ComputeNodeResize>(_resize, "compute/ComputeNodeResize");
    createNode<ComputeNodeVolcanoRecognition>(_volcano, "compute/ComputeNodeVolcanoRecognition");
    createNode<ComputeNodeBlur>(_postBlur, "compute/ComputeNodeBlur");
    createNode<ComputeNodeMinResize>(_minResize, "compute/ComputeNodeMinResize");

    createNode<ComputeNodeDepthSensor>(_depthSensor, "compute/ComputeNodeDepthSensor");

    PropertiesSystem::End();
}

void ComputeGraphCore::Initialize(InputFrameStream* stream)
{
    _sensor->SetInputStream(stream);

    _crop->SetInput(GetRootNode());
    _holesFilter->SetInput(_crop);

    // branch 1
    _minResize->SetInput(_holesFilter);
    _blackHole->SetInput(_minResize);

    // branch 2
    _delay->SetInput(_holesFilter);
    _delay->SetMotionMask(_blackHole->GetOutput());

    _volcano->SetInput(_delay);

    _postBlur->SetInput(_delay);

    AddCalculationGraph(GetRootNode());
}

GtComputeNodeBase* ComputeGraphCore::GetRootNode() const
{
    return _sensor;
}
