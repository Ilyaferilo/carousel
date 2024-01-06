#include "carousel.h"
#include "qpainter.h"
#include <QGraphicsSceneWheelEvent>
#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <qbrush.h>
#include <qfontmetrics.h>
#include <qgraphicsitem.h>
#include <QGraphicsBlurEffect>
#include <qpoint.h>
#include <qpropertyanimation.h>
#include "circle-list.h"

namespace {

constexpr qreal linePenW = 3;
constexpr qreal radius = 5;

constexpr int itemWidth{150};
constexpr int itemHeight{50};

constexpr int centerItemWidth{itemWidth + 30};
constexpr int centerItemHeight = itemHeight * 1.1;

void updateAnimation(Carousel::ItemElement e, QPointF newPos)
{
    e.moveAnimation->setStartValue(e.item->pos());
    e.moveAnimation->setEndValue(newPos);
}

QGraphicsEffect* makeBlur()
{
    auto blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(12);
    return blur;
}

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
  , m_groupAnimation(new QParallelAnimationGroup(this))
  , m_footer(new QGraphicsRectItem(QRectF(0, 500, 1500, centerItemHeight)))
  , m_top(new QGraphicsRectItem(QRectF(0, 0, 1500, centerItemHeight)))
{
    setSceneRect(0, 0, 200, 500);

    m_centerItem = new CenterItem();

    // footer->setBrush(QBrush(Qt::darkCyan));
    m_footer->setPen(Qt::NoPen);
    m_top->setPen(Qt::NoPen);

    m_footer->setGraphicsEffect(makeBlur());
    m_top->setGraphicsEffect(makeBlur());

    m_centerItem->setZValue(1);
    m_footer->setZValue(1);
    m_top->setZValue(1);

    addItem(m_centerItem);
    addItem(m_footer);
    addItem(m_top);
}

void Carousel::setBackground(QBrush brush)
{
    setBackgroundBrush(brush);
    m_footer->setBrush(brush);
    m_top->setBrush(brush);
}

void Carousel::setSceneRectangle(QRectF rect)
{
    const auto oldPos = m_centerItem->pos();
    setSceneRect(rect);
    const auto center = rect.center().x() - itemWidth / 2;
    m_centerItem->setPos(rect.center().x() - centerItemWidth / 2, rect.center().y() - centerItemHeight / 2);
    const auto diffY = m_centerItem->pos().y() - oldPos.y();
    for (ItemElement i : qAsConst(m_items.list())) {
        i.item->setPos(center, i.item->pos().y() + diffY);
    }
    m_footer->setRect(0, rect.bottom() - centerItemHeight, rect.width() + itemWidth, centerItemHeight * 2);
    m_top->setRect(0, -centerItemHeight, rect.width() + itemWidth, centerItemHeight * 2);
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

void Carousel::add(QGraphicsObject* item)
{
    addItem(item);
    auto* a = new QPropertyAnimation(item, "pos", this);
    m_groupAnimation->addAnimation(a);
    m_items.push(ItemElement{item, a});
    // a->setStartValue(item->pos());
    // a->setEndValue(m_centerItem->pos());
    // a->setEasingCurve(QEasingCurve::OutElastic);
    // a->setDuration(2000);
    // a->start(QAbstractAnimation::DeletionPolicy::DeleteWhenStopped);
    // connect(a, &QPropertyAnimation::finished, this, &Carousel::replaceItems);
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
    const auto& xl = m_items.list();
    const auto itemsCount = xl.size();
    if (itemsCount == 0) {
        return;
    }
    const auto centerX = sceneRect().center().x() - itemWidth / 2;
    int activeItem = itemsCount / 2;
    const QPointF activePoint(centerX, sceneRect().center().y() - centerItemHeight / 2 + linePenW);
    updateAnimation(xl[activeItem], activePoint);
    // xl[activeItem]->setPos(activePoint);
    for (int i = activeItem - 1, y = activePoint.y() - (itemHeight + m_padding); i >= 0;
         --i, y -= itemHeight + m_padding) {
        updateAnimation(xl[i], QPointF(centerX, y));
        // xl[i]->setPos(centerX, y);
    }
    for (int i = activeItem + 1, y = activePoint.y() + (itemHeight + m_padding); i < itemsCount;
         ++i, y += itemHeight + m_padding) {
        updateAnimation(xl[i], QPointF(centerX, y));
        // xl[i]->setPos(centerX, y);
    }
    m_groupAnimation->start();
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
    auto shift = painter->fontMetrics().size(0, m_name).width() / 2;
    painter->drawText(r.center() - QPointF(shift, 0), m_name);
}

void Carousel::setPadding(int newPadding)
{
    const bool isNewPaddingValid = newPadding >= 1 && newPadding < 300;
    if (m_padding == newPadding || !isNewPaddingValid) {
        return;
    }
    m_padding = newPadding;
    emit paddingChanged(m_padding);
}

int Carousel::padding() const
{
    return m_padding;
}
