#include "userdata.h"

// SearchInfo::SearchInfo(int _uid, QString name, QString nick, QString desc, int sex,QString icon)
// {
//     _uid=_uid;
//     _name=name;
//     _nick=nick;
//     _desc=desc;
//     _sex=sex;
//     _icon=icon;
// }

SearchInfo::SearchInfo(){
    _uid=-1;
    _name="";
    _nick="";
    _desc="";
    _sex=-1;
    _icon="";
}
AddFriendApply::AddFriendApply(int from_uid, QString name, QString desc, QString icon, QString nick, int sex):
    _from_uid(from_uid),_name(name),_desc(desc),_icon(icon),_nick(nick),_sex(sex)
{

}
