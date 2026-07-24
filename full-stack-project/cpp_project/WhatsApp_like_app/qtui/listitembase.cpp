#include "listitembase.h"
#include <QStyleOption>
#include <QPainter>

ListItemBase::ListItemBase(QWidget *parent) {}

void ListItemBase::SetItem(ListItemType type)
{
    _item_type=type;
}

ListItemType ListItemBase::getItem()
{
    return _item_type;
}

void ListItemBase::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

