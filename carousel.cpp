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
#include <qparallelanimationgroup.h>
#include <qpoint.h>
#include <qpropertyanimation.h>
#include <QKeyEvent>
#include "circle-list.h"

namespace {

constexpr qreal linePenW = 3;
constexpr qreal radius = 5;

constexpr int itemWidth{150};
constexpr int itemHeight{50};

constexpr int centerItemWidth{itemWidth + 30};
constexpr int centerItemHeight = itemHeight * 1.1;

QGraphicsEffect* makeBlur()
{
    auto blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(12);
    return blur;
}

}   // namespace

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
        painter->setRenderHint(QPainter::Antialiasing);

        painter->setPen(pen);
        painter->drawLine(0, 0, centerItemWidth, 0);
        painter->drawLine(0, centerItemHeight, centerItemWidth, centerItemHeight);
    }
};

Carousel::Carousel(QWidget* parent)
  : QGraphicsScene(parent)
  , m_centerItem(new CenterItem())
  , m_groupAnimation(new QParallelAnimationGroup(this))
  , m_footer(new QGraphicsRectItem(QRectF(0, 500, 1500, centerItemHeight)))
  , m_top(new QGraphicsRectItem(QRectF(0, 0, 1500, centerItemHeight)))
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
        if (m_rotateQueue != 0) {
            doRotate(m_rotateQueue);
            m_rotateQueue = 0;
        }
    });

    setSceneRectangle(QRectF(0, 0, 500, 800));
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
    for (const auto& i : qAsConst(m_items.list())) {
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
    item->setAcceptedMouseButtons(Qt::MouseButton::LeftButton);
    addItem(item);
    auto* moveAnimation = new QPropertyAnimation(item, "pos", this);
    auto* scaleAnimation = new QPropertyAnimation(item, "scale", this);

    auto g = new QParallelAnimationGroup(this);
    g->addAnimation(moveAnimation);
    g->addAnimation(scaleAnimation);
    m_groupAnimation->addAnimation(g);
    m_items.push(ItemElement{item, moveAnimation, scaleAnimation});
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

    const auto oldW = e.item->sceneBoundingRect().width();
    newPos.rx() += (oldW - oldW * scale) / 2;
    e.moveAnimation->setEndValue(newPos);

    e.scaleAnimation->setStartValue(e.item->scale());
    e.scaleAnimation->setEndValue(scale);
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
            int c = 0;
            const auto& lst = m_items.list();
            const auto count = lst.size();
            for (int i = 0; i < count; i++) {
                if (lst.at(i).item == item) {
                    event->accept();
                    m_items.rotate(count / 2 - i);
                    replaceItems();
                    return;
                }
            }
        }
        m_lastClickPos = event->scenePos();
    }
}

void Carousel::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    return;
    // TODO: запомнить предыдущие позиции элементов и перемещать относительно них
    const auto diffPoint = event->scenePos() - m_lastClickPos;
    if (diffPoint.manhattanLength() > 3) {
        for (const auto& i : qAsConst(m_items.list())) {
            i.item->moveBy(0, diffPoint.y());
        }
        qDebug() << diffPoint.y();
    }
}

void Carousel::replaceItems()
{
    const auto& xl = m_items.list();
    const auto itemsCount = xl.size();
    if (itemsCount == 0) {
        return;
    }
    const auto centerX = sceneRect().center().x() - itemWidth / 2;
    int activeItem = itemsCount / 2;
    const QPointF activePoint(centerX, sceneRect().center().y() - centerItemHeight / 2 + linePenW);
    updateAnimation(xl[activeItem], activePoint, 1.0);
    qreal scale = 1 - 0.1;
    for (int i = activeItem - 1, y = activePoint.y() - (itemHeight + m_padding); i >= 0;
         --i, y -= itemHeight + m_padding) {
        updateAnimation(xl[i], QPointF(centerX, y), scale);
        scale -= 0.1;
    }
    scale = 1 - 0.1;
    for (int i = activeItem + 1, y = activePoint.y() + (itemHeight + m_padding); i < itemsCount;
         ++i, y += itemHeight + m_padding) {
        updateAnimation(xl[i], QPointF(centerX, y), scale);
        scale -= 0.1;
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
    painter->setRenderHint(QPainter::Antialiasing);
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
