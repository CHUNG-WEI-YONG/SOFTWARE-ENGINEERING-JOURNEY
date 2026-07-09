#ifndef CLICKEDBUTTON_H
#define CLICKEDBUTTON_H
#include "QPushButton"

class ClickedButton:public QPushButton
{
    Q_OBJECT
public:
    ClickedButton(QWidget *parent =nullptr);
    ~ClickedButton();
    void SetState(QString normal,QString hover,QString press);
private:
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

    QString _normal;
    QString _hover;
    QString _press;
};

#endif // CLICKEDBUTTON_H
