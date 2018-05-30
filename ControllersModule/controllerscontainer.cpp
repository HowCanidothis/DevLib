#include "controllerscontainer.h"
#include "controllerbase.h"
#include "controllerssystem.h"

ControllersContainer::ControllersContainer(QObject* parent)
    : QObject(parent)
    , _currentController(nullptr)
{

}

ControllersContainer::~ControllersContainer()
{

}

void ControllersContainer::SetCurrent(ControllerBase* controller) {
    Q_ASSERT(_currentController != nullptr);
    if(_currentController != controller) {
        ControllerBase* cp = findCommonParent(controller, _currentController);
        ControllerBase* c = _currentController;
        while(c != cp) {
            c->ResetCommandsChain();
            c->Abort();
            c = c->GetParentController();
        }
        _currentController = controller;
    }
}

void ControllersContainer::SetCurrent(const Name& name)
{
    SetCurrent(ControllersSystem::GetController(name));
}

void ControllersContainer::Accept()
{
    if(ControllerBase* cp = _currentController->GetParentController()) {
        _currentController->Accept(cp->GetCommands());
        this->SetCurrent(cp);
    }
}

void ControllersContainer::Abort()
{
    if(ControllerBase* cp = _currentController->GetParentController()) {
        this->SetCurrent(cp);
    }
}

void ControllersContainer::Undo()
{
    _currentController->Undo();
}

void ControllersContainer::Redo()
{
    _currentController->Redo();
}

void ControllersContainer::Draw(DrawEngineBase* engine)
{
    callFunctionRecursivly(&ControllerBase::Draw, engine);
}

void ControllersContainer::MouseMoveEvent(QMouseEvent* e)
{
    callFunctionRecursivly(&ControllerBase::mouseMoveEvent, e);
}

void ControllersContainer::MousePressEvent(QMouseEvent* e)
{
    callFunctionRecursivly(&ControllerBase::mousePressEvent, e);
}

void ControllersContainer::MouseReleaseEvent(QMouseEvent* e)
{
    callFunctionRecursivly(&ControllerBase::mouseReleaseEvent, e);
}

void ControllersContainer::MouseDoubleClickEvent(QMouseEvent* e)
{
    callFunctionRecursivly(&ControllerBase::mouseDoubleClickEvent, e);
}

void ControllersContainer::WheelEvent(QWheelEvent* e)
{
    callFunctionRecursivly(&ControllerBase::wheelEvent, e);
}

void ControllersContainer::KeyPressEvent(QKeyEvent* e)
{
    callFunctionRecursivly(&ControllerBase::keyPressEvent, e);
}

void ControllersContainer::KeyReleaseEvent(QKeyEvent* e)
{
    callFunctionRecursivly(&ControllerBase::keyReleaseEvent, e);
}

void ControllersContainer::ContextMenuEvent(QMenu* menu)
{
    callFunctionRecursivly(&ControllerBase::contextMenuEvent, menu);
}

ControllerBase* ControllersContainer::findCommonParent(ControllerBase* c1, ControllerBase* c2) const
{
    Controllers crl1 = findAllParents(c1);
    Controllers crl2 = findAllParents(c2);

    auto b1 = crl1.begin();
    auto e1 = crl1.end();
    auto b2 = crl2.begin();
    auto e2 = crl2.end();

    ControllerBase* res = nullptr;

    while(b1 != e1 && b2 != e2 && *b1 == *b2) {
        res = *b1;
        b1++;
        b2++;
    }

    if(b1 != e1 && *b1 == c2) {
        res = c2;
    }
    else if(b2 != e2 && *b2 == c1) {
        res = c1;
    }

    return res;
}

ControllersContainer::Controllers ControllersContainer::findAllParents(ControllerBase* c) const
{
    Controllers res;
    if(c == nullptr) {
        return res;
    }
    while(c = c->GetParentController()) {
        res.Prepend(c);
    }
    return res;
}
