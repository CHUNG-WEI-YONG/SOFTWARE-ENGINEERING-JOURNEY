#include "listitembase.h"

ListItemBase::ListItemBase(QWidget *parent) {}

void ListItemBase::SetItem(ListItemType type)
{
    _item_type=type;
}

ListItemType ListItemBase::getItem()
{
    return _item_type;
}

