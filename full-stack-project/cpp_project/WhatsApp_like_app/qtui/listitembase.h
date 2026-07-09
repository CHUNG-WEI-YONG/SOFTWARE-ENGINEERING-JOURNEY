#ifndef LISTITEMBASE_H
#define LISTITEMBASE_H
#include <QWidget>
#include "global.h"

class ListItemBase:public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget *parent=nullptr);
    void SetItem(ListItemType);
    ListItemType getItem();

private:
    ListItemType _item_type;

public slots:

signals:
};

#endif // LISTITEMBASE_H
