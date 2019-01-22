#ifndef MEMORYVIEWCONTROL_H
#define MEMORYVIEWCONTROL_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

namespace Ui {
class MemoryViewControl;
}

class MemoryViewControl : public QWidget
{
    Q_OBJECT

public:
    explicit MemoryViewControl(QWidget *parent = nullptr);
    ~MemoryViewControl();

public Q_SLOTS:
    void AboutToBeReset();
    void Reset();

private:
    Ui::MemoryViewControl *ui;

    PointerPropertyPtr<class DbDatabase> m_currentDatabase;

    class ModelMemoryTree* model() const;
};

#endif // MEMORYVIEWCONTROL_H
