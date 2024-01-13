#include "qpainter.h"
#include <QGraphicsSceneWheelEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <qbrush.h>
#include <qfontmetrics.h>
#include <qgraphicsitem.h>
#include <QGraphicsBlurEffect>
#include <qparallelanimationgroup.h>
#include <qpoint.h>
#include <qpropertyanimation.h>
#include <QKeyEvent>
#include <qsize.h>

#include "carousel/carousel.h"
#include "carousel/circle-list.h"

namespace {

constexpr qreal linePenW = 3;
constexpr int hideHeight{50};

QGraphicsEffect* makeBlur()
{
    auto blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(12);
    return blur;
}

class CenterItem : public QGraphicsItem
{
    QSizeF m_size{180, 56};

public:
    void setSize(QSizeF newSize)
    {
        m_size = newSize;
        update();
    }

    QRectF boundingRect() const override
    {
        return {-linePenW / 2, -linePenW / 2, m_size.width() + linePenW, m_size.height() + linePenW};
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) override
    {
        QPen pen(QBrush(Qt::white), linePenW, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(pen);
        painter->drawLine(0, 0, m_size.width(), 0);
        painter->drawLine(0, m_size.height(), m_size.width(), m_size.height());
    }
};

}   // namespace

Carousel::Carousel(QObject* parent)
  : QGraphicsScene(parent)
  , m_centerItem(new CenterItem())
  , m_groupAnimation(new QParallelAnimationGroup(this))
  , m_footer(new QGraphicsRectItem(QRectF(0, 500, 1500, hideHeight)))
  , m_top(new QGraphicsRectItem(QRectF(0, 0, 1500, hideHeight)))
{
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

    connect(m_groupAnimation, &QParallelAnimationGroup::finished, this, [this] {
        setFocusItem(m_items.list().at(m_items.list().size() / 2).item);

        if (m_rotateQueue != 0) {
            doRotate(m_rotateQueue);
            m_rotateQueue = 0;
        }
    });
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
    const auto centerPoint = rect.center();
    const auto centerX = centerPoint.x() - m_itemSize.width() / 2;
    const auto centerItemSize = m_centerItem->boundingRect().size();
    m_centerItem->setPos(centerPoint.x() - centerItemSize.width() / 2, centerPoint.y() - centerItemSize.height() / 2);
    const auto diffY = m_centerItem->pos().y() - oldPos.y();
    for (const auto& i : qAsConst(m_items.list())) {
        i.item->setPos(centerX + (m_itemSize.width() - m_itemSize.width() * i.item->scale()) / 2,
                       i.item->pos().y() + diffY);
    }
    m_top->setRect(-10, -hideHeight / 2, rect.width() + m_itemSize.width(), hideHeight);
    m_footer->setRect(-10, rect.bottom() - hideHeight / 2, rect.width() + m_itemSize.width(), hideHeight);
}

void Carousel::setActiveItem(QGraphicsItem* item)
{
    const auto& lst = m_items.list();
    const auto count = lst.size();
    for (int i = 0; i < count; ++i) {
        if (lst.at(i).item == item) {
            m_items.rotate(count / 2 - i);
            replaceItems();
            return;
        }
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

void Carousel::add(QGraphicsObject* item)
{
    addItem(item);
    item->setFlags(QGraphicsItem::ItemIsFocusable);

    auto* moveAnimation = new QPropertyAnimation(item, "pos", this);
    auto* scaleAnimation = new QPropertyAnimation(item, "scale", this);
    auto* opacityAnimation = new QPropertyAnimation(item, "opacity", this);

    auto* g = new QParallelAnimationGroup(this);
    g->addAnimation(moveAnimation);
    g->addAnimation(scaleAnimation);
    g->addAnimation(opacityAnimation);

    m_groupAnimation->addAnimation(g);
    m_items.push(ItemElement{item, moveAnimation, scaleAnimation, opacityAnimation});
    setActiveItem(focusItem());
}

void Carousel::reset()
{
    m_items.reset();
    replaceItems();
}

void Carousel::wheelEvent(QGraphicsSceneWheelEvent* event)
{
    const int rotateDirection = event->delta() > 0 ? 1 : -1;
    doRotate(rotateDirection);
}

void Carousel::doRotate(int count)
{
    if (m_groupAnimation->state() == QAbstractAnimation::Running) {
        m_rotateQueue += count;
        return;
    }
    m_items.rotate(count);
    replaceItems();
}

void Carousel::updateAnimation(const Carousel::ItemElement& e, QPointF newPos, qreal scale)
{
    e.moveAnimation->setKeyValues({});
    e.moveAnimation->setStartValue(e.item->pos());

    newPos.rx() += (m_itemSize.width() - m_itemSize.width() * scale) / 2;
    e.moveAnimation->setEndValue(newPos);

    e.scaleAnimation->setStartValue(e.item->scale());
    e.scaleAnimation->setEndValue(scale);

    e.opacityAnimation->setStartValue(e.item->opacity());
    e.opacityAnimation->setEndValue(scale);
    if (qAbs(e.item->pos().y() - newPos.y()) > sceneRect().height() * 0.55) {
        //TODO: use QSequentialAnimationGroup gnu;
        if (e.item->pos().y() > newPos.y()) {
            e.moveAnimation->setKeyValueAt(0.5, QPointF{newPos.x(), sceneRect().bottom()});
            e.moveAnimation->setKeyValueAt(0.50001, QPointF{newPos.x(), sceneRect().top()});
        } else {
            e.moveAnimation->setKeyValueAt(0.5, QPointF{newPos.x(), sceneRect().top()});
            e.moveAnimation->setKeyValueAt(0.50001, QPointF{newPos.x(), sceneRect().bottom()});
        }
    }
}

void Carousel::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        doRotate(1);
        break;
    case Qt::Key_Down:
        doRotate(-1);
        break;
    default:
        break;
    };
}

void Carousel::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        if (auto* item = itemAt(event->scenePos(), {})) {
            event->accept();
            setActiveItem(item);
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void Carousel::replaceItems()
{
    const auto& xl = m_items.list();
    const int itemsCount = static_cast<int>(xl.size());
    if (itemsCount == 0) {
        return;
    }
    const auto centerX = sceneRect().center().x() - m_itemSize.width() / 2;
    const int activeItem = itemsCount / 2;
    const QPointF activePoint(centerX, sceneRect().center().y() - m_centerItem->boundingRect().height() / 2 + linePenW);
    updateAnimation(xl[activeItem], activePoint, 1.0);
    const qreal step = 0.1;
    const auto itemHeight = m_itemSize.height();
    // go up
    {
        qreal scale = 1 - step;
        qreal y = activePoint.y() - (itemHeight + m_margin);
        for (int i = activeItem - 1; i >= 0; --i) {
            updateAnimation(xl[i], QPointF(centerX, y), scale);
            scale -= step;
            y -= itemHeight + m_margin;
        }
    }
    // go down
    {
        qreal scale = 1 - step;
        qreal y = activePoint.y() + (itemHeight + m_margin);
        for (int i = activeItem + 1; i < itemsCount; ++i) {
            updateAnimation(xl[i], QPointF(centerX, y), scale);
            scale -= step;
            y += itemHeight + m_margin;
        }
    }
    m_groupAnimation->start();
}

void Carousel::setMargin(int newMargin)
{
    const bool isNewPaddingValid = newMargin >= 1 && newMargin < 300;
    if (m_margin == newMargin || !isNewPaddingValid) {
        return;
    }
    m_margin = newMargin;
    replaceItems();
}

int Carousel::margin() const
{
    return m_margin;
}

const QSizeF& Carousel::itemSize() const
{
    return m_itemSize;
}

void Carousel::setItemSize(const QSizeF& newItemSize)
{
    m_itemSize = newItemSize;
    auto centerItemSize = newItemSize;
    centerItemSize.rwidth() += 30;
    centerItemSize.rheight() += 6;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    static_cast<CenterItem*>(m_centerItem)->setSize(centerItemSize);
    replaceItems();
}
