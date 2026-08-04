#include "userdata.h"

SearchInfo::SearchInfo(int _uid, QString name, QString nick, QString desc, int sex,QString icon)
{
    _uid=_uid;
    _name=name;
    _nick=nick;
    _desc=desc;
    _sex=sex;
    _icon=icon;
}

SearchInfo::SearchInfo(){
    _uid=-1;
    _name="";
    _nick="";
    _desc="";
    _sex=-1;
    _icon="";
}