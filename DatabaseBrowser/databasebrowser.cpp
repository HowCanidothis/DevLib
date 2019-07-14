#include "databasebrowser.h"
#include "ui_databasebrowser.h"

#include <QFileDialog>
#include <QToolButton>

#include <SharedModule/External/qtqssreader.h>
#include <DatabaseEngine/internal.hpp>

#include "Models/modelmemorytree.h"
#include "Models/dbmodeltabletable.h"
#include "databaseactionsscope.h"

DatabaseBrowser::DatabaseBrowser(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DatabaseBrowser)
    , m_settingsDialogGeometry("ProgramSettings")
    , m_qssFile("Style/Path", "")
    , m_databaseActionsScope(new DatabaseActionsScope())
{
    m_databaseActionsScope->CreateActions();

    ui->setupUi(this);

    connect(ui->HeadersVC, SIGNAL(modelAboutToBeChanged()), ui->TablesVC, SLOT(AboutToBeReset()));
    connect(ui->HeadersVC, SIGNAL(modelChanged()), ui->TablesVC, SLOT(Reset()));
    connect(ui->HeadersVC, SIGNAL(modelAboutToBeChanged()), ui->MemoryVC, SLOT(AboutToBeReset()));
    connect(ui->HeadersVC, SIGNAL(modelChanged()), ui->MemoryVC, SLOT(Reset()));
    connect(ui->TablesVC, SIGNAL(modelAboutToBeChanged()), ui->MemoryVC, SLOT(AboutToBeReset()));
    connect(ui->TablesVC, SIGNAL(modelChanged()), ui->MemoryVC, SLOT(Reset()));

    PropertiesSystem::Load("app.ini", PropertiesSystem::Global);

    m_qssReader = new QtQSSReader;
    m_qssReader->SetEnableObserver(true);
    m_qssFile.Subscribe([this]{
        m_qssReader->Install(m_qssFile);
    });

    for(auto* action : m_databaseActionsScope->GetActions())
    {
        auto toolBtn = new QToolButton();
        toolBtn->setDefaultAction(action);
        toolBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        ui->ToolBarLayout->addWidget(toolBtn);
    }
}

DatabaseBrowser::~DatabaseBrowser()
{
    PropertiesSystem::Save("app.ini", PropertiesSystem::Global);
    delete ui;
}

void DatabaseBrowser::showEvent(QShowEvent* )
{
    m_qssReader->Install(m_qssFile);
}
