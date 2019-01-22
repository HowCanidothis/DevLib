#include "memoryviewcontrol.h"
#include "ui_memoryviewcontrol.h"

#include <DatabaseEngine/internal.hpp>

#include "Models/modelmemorytree.h"

MemoryViewControl::MemoryViewControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MemoryViewControl)
    , m_currentDatabase("Database/Current/Database", PropertiesSystem::Global)
{
    ui->setupUi(this);

    m_currentDatabase.Subscribe([this] {
        auto model = ui->MemoryView->model();
        if(model != nullptr) {
            ui->MemoryView->setModel(nullptr);
            delete model;
        }

        if(m_currentDatabase != nullptr) {
            ui->MemoryView->setModel(new ModelMemoryTree(m_currentDatabase->GetMemory()));
            ui->MemoryView->expandAll();
        }
    });
}

MemoryViewControl::~MemoryViewControl()
{
    delete ui;
}

void MemoryViewControl::AboutToBeReset()
{
    emit model()->layoutAboutToBeChanged();
}

void MemoryViewControl::Reset()
{
    model()->Reset();
    ui->MemoryView->reset();
    ui->MemoryView->expandAll();
}

ModelMemoryTree* MemoryViewControl::model() const
{
    return reinterpret_cast<ModelMemoryTree*>(ui->MemoryView->model());
}
