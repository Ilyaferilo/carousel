#pragma once

#include "circle-list.h"
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <qbrush.h>

class QPropertyAnimation;
class QParallelAnimationGroup;

class Carousel : public QGraphicsScene
{
    Q_OBJECT
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
public:
    Carousel(QWidget* parent = nullptr);

    void setSceneRectangle(QRectF rect);
    void setBackground(QBrush brush);

    void setActive(int itemNumber);

    void add(QGraphicsObject* item);

    void reset();

    void setPadding(int newPadding);
    int padding() const;

    struct ItemElement
    {
        QGraphicsObject* item;
        QPropertyAnimation* moveAnimation;
    };
signals:
    void paddingChanged(int);

protected:
    void wheelEvent(QGraphicsSceneWheelEvent* event) override;

private:
    void replaceItems() const;

    CircleList<ItemElement> m_items;
    QGraphicsItem* m_centerItem;
    int m_padding{1};
    QParallelAnimationGroup* m_groupAnimation;
    QGraphicsRectItem* m_footer;
    QGraphicsRectItem* m_top;
};

class Item : public QGraphicsObject
{
    Q_OBJECT
    // QGraphicsItem interface

public:
    const QString m_name;
    Item(const QString& name);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};
