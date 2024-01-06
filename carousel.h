#ifndef CAROUSEL_H
#define CAROUSEL_H

#include "circle-list.h"
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsObject>

class Carousel : public QGraphicsScene
{
    Q_OBJECT
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
public:
    Carousel(QWidget* parent = nullptr);

    void setSceneRectangle(QRectF rect);

    void setActive(int itemNumber);
    void add();
    void reset();

    void setPadding(int newPaddingREAD);

    int padding() const;

signals:
    void paddingChanged(int);

protected:
    void wheelEvent(QGraphicsSceneWheelEvent* event) override;

private:
    void replaceItems() const;

    CircleList<QGraphicsItem*> m_items;
    QGraphicsItem* m_centerItem;
    int m_padding{1};
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

#endif   // CAROUSEL_H
