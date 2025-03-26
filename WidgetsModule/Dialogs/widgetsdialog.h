#ifndef WIDGETSDIALOG_H
#define WIDGETSDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>

#include "WidgetsModule/widgetsdeclarations.h"

namespace Ui {
class WidgetsDialog;
}

class WidgetsDialog : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
public:
    explicit WidgetsDialog(QWidget *parent = nullptr);
    ~WidgetsDialog();

    template<class T>
    T* GetView() const { return reinterpret_cast<T*>(m_content); }
    QAbstractButton* GetButton(qint32 i) const { return m_buttons.at(i); }

    void Initialize(const std::function<void (qint32)>& onDone = nullptr, const std::function<void (const QVector<QAbstractButton*>&)>& handler = nullptr);
    void SetHeaderText(const FTranslationHandler& text);
    void SetContent(QWidget* view);
    QAbstractButton* AddButton(const WidgetsDialogsManagerButtonStruct& b);

private:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent* e) override;
    // QDialog interface
public slots:
    void done(int) override;

private:
    Ui::WidgetsDialog *ui;
    QVector<QAbstractButton*> m_buttons;
    QHash<QAbstractButton*, ButtonRole> m_roles;
    std::function<void (qint32)> m_onDone;
    QWidget* m_content;
};

#endif // WidgetsDialog_H
