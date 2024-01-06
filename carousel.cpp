#include "carousel.h"
#include "qpainter.h"
#include <QGraphicsSceneWheelEvent>
#include <QDebug>
#include <utility>
#include <QPropertyAnimation>
#include "circle-list.h"

namespace {

constexpr qreal linePenW = 3;
constexpr qreal radius = 5;

constexpr int itemWidth{150};
constexpr int itemHeight{50};

constexpr int centerItemWidth{itemWidth + 30};
constexpr int centerItemHeight = itemHeight * 1.1;

int counter = 0;

}   // namespace

QDebug operator<<(QDebug dbg, const CircleList<QGraphicsItem*>& value)
{
    for (auto&& v : value.list()) {
        dbg << static_cast<Item*>(v)->m_name << ", ";
    }
    return dbg;
}

class CenterItem : public QGraphicsItem
{
    // QGraphicsItem interface

public:
    QRectF boundingRect() const override
    {
        return QRectF(-linePenW / 2, -linePenW / 2, centerItemWidth + linePenW, centerItemHeight + linePenW);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    {
        QPen pen(QBrush(Qt::darkRed), linePenW, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
        painter->setPen(pen);
        painter->drawLine(0, 0, centerItemWidth, 0);
        painter->drawLine(0, centerItemHeight, centerItemWidth, centerItemHeight);
    }
};

Carousel::Carousel(QWidget* parent)
  : QGraphicsScene(parent)
{
    setBackgroundBrush(QBrush(Qt::darkCyan));
    const int x = 100;   // this->sceneRect().center().x();
    for (size_t i = 0; i < 5; i++) {
        auto it = new Item(QString::number(i));
        m_items.push(it);
        addItem(it);
        it->setZValue(-1);
        it->setPos(x, (i * itemHeight * 1.2));
    }

    counter = m_items.list().size();

    setSceneRect(0, 0, 200, 500);

    m_centerItem = new CenterItem();
    m_centerItem->setZValue(1);
    addItem(m_centerItem);
}

void Carousel::setSceneRectangle(QRectF rect)
{
    const auto oldPos = m_centerItem->pos();
    setSceneRect(rect);
    const auto center = rect.center().x() - itemWidth / 2;
    m_centerItem->setPos(rect.center().x() - centerItemWidth / 2, rect.center().y() - centerItemHeight / 2);
    const auto diffY = m_centerItem->pos().y() - oldPos.y();
    for (auto i : qAsConst(m_items.list())) {
        i->setPos(center, i->pos().y() + diffY);
    }
}

void Carousel::setActive(int itemNumber)
{
    const auto itemsCount = m_items.list().size();
    if (itemNumber >= itemsCount) {
        return;
    }
    m_items.reset();
    m_items.rotate(itemsCount / 2 - itemNumber);
    replaceItems();
}

void Carousel::add()
{
    auto it = new Item(QString::number(counter++));
    m_items.push(it);
    addItem(it);
    auto a = new QPropertyAnimation(it, "pos");
    a->setStartValue(it->pos());
    a->setEndValue(m_centerItem->pos());
    a->setEasingCurve(QEasingCurve::OutElastic);
    a->setDuration(2000);
    a->start(QAbstractAnimation::DeletionPolicy::DeleteWhenStopped);
    connect(a, &QPropertyAnimation::finished, this, &Carousel::replaceItems);
}

void Carousel::reset()
{
    m_items.reset();
    replaceItems();
}

void Carousel::wheelEvent(QGraphicsSceneWheelEvent* event)
{
    if (event->delta() > 0) {
        m_items.rotateR();
    } else {
        m_items.rotateL();
    }
    replaceItems();
}

void Carousel::replaceItems() const
{
    const auto centerX = sceneRect().center().x() - itemWidth / 2;

    const auto& xl = m_items.list();
    auto itemsCount = xl.size();
    int activeItem = itemsCount / 2;
    const QPointF activePoint(centerX, sceneRect().center().y() - centerItemHeight / 2 + linePenW);
    xl[activeItem]->setPos(activePoint);

    for (int i = activeItem - 1, y = activePoint.y() - (itemHeight + m_padding); i > 0;
         --i, y -= itemHeight + m_padding) {
        xl[i]->setPos(centerX, y);
    }
    for (int i = activeItem + 1, y = activePoint.y() + (itemHeight + m_padding); i < itemsCount;
         ++i, y += itemHeight + m_padding) {
        xl[i]->setPos(centerX, y);
    }
}

Item::Item(const QString& name)
  : m_name(name)
{}

QRectF Item::boundingRect() const
{
    return QRectF(-radius, -radius, itemWidth + radius, itemHeight + radius);
}

void Item::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QRectF r(0, 0, itemWidth, itemHeight);
    painter->setBrush(QBrush(Qt::darkGray));
    painter->drawRoundedRect(r, radius, radius);
    painter->drawText(r.center(), m_name);
}

void Carousel::setPadding(int newPaddingREAD)
{
    if (m_padding == newPaddingREAD)
        return;
    m_padding = newPaddingREAD;
    emit paddingChanged(m_padding);
}

int Carousel::padding() const
{
    return m_padding;
}
