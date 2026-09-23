#ifndef USERINFOPAGE_H
#define USERINFOPAGE_H

#include <QWidget>

namespace Ui {
class UserInfoPage;
}

class UserInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserInfoPage(QWidget *parent = nullptr);
    ~UserInfoPage();

private slots:
    void on_upload_btn_clicked();

private:
    Ui::UserInfoPage *ui;
};

#endif // USERINFOPAGE_H
