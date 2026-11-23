#ifndef HORIZONTALSCROLLSTRIP_H
#define HORIZONTALSCROLLSTRIP_H

#include <QWidget>

#include <PropertiesModule/internal.hpp>

class QScrollArea;
class QPushButton;
class QHBoxLayout;

// Горизонтальная полоса виджетов, обрезаемая по ширине.
// Скроллбары скрыты, прокрутка - стрелками по краям (опционально колесом).
// Стилей нет: цвета, размеры и иконки задаёт пользователь (QSS по objectName кнопок).
class HorizontalScrollStrip : public QFrame
{
    Q_OBJECT
    using Super = QFrame;
    Q_PROPERTY(qint32 spacing MEMBER Spacing);

public:
    HorizontalScrollStrip(QWidget* parent = nullptr);

    void Clear();                        // удаляет все элементы (deleteLater)
    void AddWidget(QWidget* widget);     // виджет становится дочерним полосы
    void InsertWidget(qint32 index, QWidget* widget); // вставка по индексу в раскладку
    void RemoveWidget(QWidget* widget);  // снимает с полосы, владение у вызывающего

    // true (по умолчанию): стрелки видны только при переполнении.
    // false: стрелки видны всегда, на краях неактивны.
    void SetAutoHideArrows(bool autoHide);
    bool IsAutoHideArrows() const { return m_autoHideArrows; }

    // Горизонтальная прокрутка колесом мыши (через WidgetScrollAreaWrapper::AddScrollByWheel).
    void EnableWheelScrolling();

    QPushButton* GetLeftButton() const { return m_leftButton; }
    QPushButton* GetRightButton() const { return m_rightButton; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    LocalPropertyInt Spacing;            // расстояние между элементами, по умолчанию 8

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void scrollByStep(qint32 direction); // -1 влево, +1 вправо
    void updateArrowsVisibility();       // единственное место политики видимости стрелок
    void updateArrowsEnabled();
    bool isOverflow() const;

    QPushButton* m_leftButton;
    QPushButton* m_rightButton;
    QScrollArea* m_scrollArea;
    QWidget* m_content;
    QHBoxLayout* m_layout;
    bool m_autoHideArrows;
};

#endif // HORIZONTALSCROLLSTRIP_H
