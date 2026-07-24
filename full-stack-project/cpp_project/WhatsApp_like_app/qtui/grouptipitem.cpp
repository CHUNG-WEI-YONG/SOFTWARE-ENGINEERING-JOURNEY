#include "grouptipitem.h"
#include "ui_grouptipitem.h"
#include "listitembase.h"

GroupTipItem::GroupTipItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::GroupTipItem),_str("")
{
    ui->setupUi(this);
    SetItem(ListItemType::Group_Tip_Item);
}

GroupTipItem::~GroupTipItem()
{
    delete ui;
}

void GroupTipItem::SetGroupTip(QString str)
{
    ui->label->setText(str);
    _str=str;
}

QSize GroupTipItem::sizeHint() const
{
    return QSize(250, 25); // 返回自定义的尺寸
}