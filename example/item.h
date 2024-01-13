#pragma once

#include <QGraphicsObject>

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
