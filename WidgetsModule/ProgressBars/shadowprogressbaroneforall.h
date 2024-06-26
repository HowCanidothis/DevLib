#ifndef SHADOWPROGRESSBARONEFORALL_H
#define SHADOWPROGRESSBARONEFORALL_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>
#include <SharedModule/internal.hpp>

namespace Ui {
class ShadowProgressBarOneForAll;
}

class ShadowProgressBarOneForAll : public QWidget
{
    Q_OBJECT

public:
    explicit ShadowProgressBarOneForAll(QWidget *parent = nullptr);
    ~ShadowProgressBarOneForAll();

    CommonDispatcher<const QString&> OnLabelChanged;

private:
    Ui::ShadowProgressBarOneForAll *ui;
    QHash<size_t, QString> m_processes;
    LocalPropertyInt64 m_value;
    LocalPropertyInt m_stepsCount;
    LocalPropertyInt m_step;
    LocalPropertyString m_label;
};

#endif // SHADOWPROGRESSBARONEFORALL_H
