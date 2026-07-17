#ifndef USERDATA_H
#define USERDATA_H
#include <QString>

// class UserData
// {
// public:
//     UserData();
// };

class SearchInfo{
public:
    SearchInfo(int uid,QString name,QString nick,QString desc,int sex);
    SearchInfo();
    int uid;
    QString _name;
    QString _nick;
    QString _desc;
    int _sex;

};

#endif // USERDATA_H
