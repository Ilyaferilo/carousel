#include "item.h"
#include <QPainter>

namespace {

constexpr qreal radius = 5;
constexpr int itemWidth{150};
constexpr int itemHeight{50};
}   // namespace

Item::Item(const QString& name)
  : m_name(name)
{
    setFlags(GraphicsItemFlag::ItemIsFocusable | GraphicsItemFlag::ItemIsSelectable);
}

QRectF Item::boundingRect() const
{
    return QRectF(-radius, -radius, itemWidth + radius, itemHeight + radius);
}
void Item::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QRectF r(0, 0, itemWidth, itemHeight);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(QBrush(0xf5eaa9));
    painter->drawRoundedRect(r, radius, radius);
    auto shift = painter->fontMetrics().size(0, m_name).width() / 2;
    painter->drawText(r.center() - QPointF(shift, 0), m_name);
}
