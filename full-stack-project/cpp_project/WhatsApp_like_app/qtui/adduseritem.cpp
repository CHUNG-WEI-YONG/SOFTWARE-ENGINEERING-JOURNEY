#include "adduseritem.h"
#include "ui_adduseritem.h"

AddUserItem::AddUserItem(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::AddUserItem)
{
    ui->setupUi(this);
    SetItem(ListItemType::Add_User_Tip_Item);
}

AddUserItem::~AddUserItem()
{
    delete ui;
}
