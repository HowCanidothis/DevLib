#include "controllerscontainer.h"

#include <QKeyEvent>

#include "controllerbase.h"

ControllersContainer::ControllersContainer(QObject* parent)
    : QObject(parent)
    , m_currentController(nullptr)
    , m_context(nullptr)
    , m_inputKeysModifiers(0)
    , m_visibilityMask(0xffffffff)
{

}

ControllersContainer::~ControllersContainer()
{

}

void ControllersContainer::SetVisibilityMask(qint32 vm)
{
    m_visibilityMask = vm;
}

void ControllersContainer::SetCurrent(ControllerBase* controller) {
    Q_ASSERT(m_currentController != nullptr);
    if(m_currentController != controller) {
        ControllerBase* cp = findCommonParent(controller, m_currentController);
        ControllerBase* c = m_currentController;
        while(c != cp) {
            c->Cancel();
            c = c->GetParentController();
        }

        Controllers applyControllers { controller };
        c = controller;
        while(c != cp) {
            c = controller->GetParentController();
            applyControllers.Append(c);
        }

        for(ControllerBase* controller : adapters::reverse(applyControllers)) {
            controller->enterEvent();
        }

        m_currentController = controller;
    }
}

void ControllersContainer::LeaveEvent()
{
    GetCurrent()->mouseLeaveEvent();
    auto* pc = GetCurrent()->GetParentController();
    if(pc != nullptr && pc->resetOnLeave()) {
        SetCurrent(pc);
    }
}

void ControllersContainer::SetCurrent(const Name& name)
{
    auto foundIt = m_controllersMap.find(name);
    Q_ASSERT(foundIt != m_controllersMap.end());
    SetCurrent(foundIt.value());
}

void ControllersContainer::Accept()
{
    m_currentController->Accept();
}

void ControllersContainer::Cancel()
{
    m_currentController->Cancel();
}

void ControllersContainer::Input()
{
    callFunctionRecursively(&ControllerBase::inputHandle, (const QSet<qint32>*) &getInputKeys(), getInputKeysModifiers());
}

void ControllersContainer::Draw(DrawEngineBase* engine)
{
    callFunctionRecursively(&ControllerBase::draw, engine);
}

void ControllersContainer::MouseMoveEvent(QMouseEvent* e)
{
    callFunctionRecursively(&ControllerBase::mouseMoveEvent, e);
}

void ControllersContainer::MousePressEvent(QMouseEvent* e)
{
    callFunctionRecursively(&ControllerBase::mousePressEvent, e);
}

void ControllersContainer::MouseReleaseEvent(QMouseEvent* e)
{
    callFunctionRecursively(&ControllerBase::mouseReleaseEvent, e);
}

void ControllersContainer::MouseDoubleClickEvent(QMouseEvent* e)
{
    callFunctionRecursively(&ControllerBase::mouseDoubleClickEvent, e);
}

void ControllersContainer::WheelEvent(QWheelEvent* e)
{
    callFunctionRecursively(&ControllerBase::wheelEvent, e);
}

void ControllersContainer::KeyPressEvent(QKeyEvent* e)
{
    getInputKeys() += e->key();
    getInputKeysModifiers() = e->modifiers();
    callFunctionRecursively(&ControllerBase::keyPressEvent, e);
}

void ControllersContainer::KeyReleaseEvent(QKeyEvent* e)
{
    getInputKeys() -= e->key();
    getInputKeysModifiers() = e->modifiers();
    callFunctionRecursively(&ControllerBase::keyReleaseEvent, e);
}

void ControllersContainer::ContextMenuEvent(QMenu* menu)
{
    callFunctionRecursively(&ControllerBase::contextMenuEvent, menu);
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

void ControllersContainer::registerController(const Name& name, ControllerBase* controller)
{
    Q_ASSERT(!m_controllersMap.contains(name));
    m_controllersMap.insert(name, controller);
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

void ControllersContainer::addMainController(ControllerBase* controller)
{
    Q_ASSERT(controller->GetParentController() == nullptr);
    m_controllers.Append(controller);
    m_currentController = (m_currentController == nullptr) ? controller : m_currentController;
}
