#pragma once

#include "circle-list.h"
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <qbrush.h>

class QPropertyAnimation;
class QParallelAnimationGroup;

class Carousel : public QGraphicsScene
{
    Q_OBJECT
    Q_PROPERTY(int margin READ margin WRITE setMargin)
    Q_PROPERTY(QSizeF itemSize READ itemSize WRITE setItemSize)
public:
    Carousel(QObject* parent = nullptr);

    void setSceneRectangle(QRectF rect);
    void setBackground(QBrush brush);

    void setActive(int itemNumber);
    void setActiveItem(QGraphicsItem* item, int indexHint = 0);

    void add(QGraphicsObject* item);

    void reset();

    void setMargin(int newMargin);
    int margin() const;
    const QSizeF& itemSize() const;
    void setItemSize(const QSizeF& newItemSize);

protected:
    void wheelEvent(QGraphicsSceneWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private slots:
    void doRotate(int count);

private:
    struct ItemElement
    {
        QGraphicsObject* item;
        QPropertyAnimation* moveAnimation;
        QPropertyAnimation* scaleAnimation;
        QPropertyAnimation* opacityAnimation;
    };

    void replaceItems();
    void updateAnimation(const ItemElement& e, QPointF newPos, qreal scale);

    CircleList<ItemElement> m_items;
    QGraphicsItem* m_centerItem;
    int m_margin{2};
    QParallelAnimationGroup* m_groupAnimation;
    QGraphicsRectItem* m_footer;
    QGraphicsRectItem* m_top;
    int m_rotateQueue{};

    QSizeF m_itemSize{150, 50};
};
