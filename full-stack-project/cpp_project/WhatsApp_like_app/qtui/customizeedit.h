#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H
#include <QLineEdit>
#include <QObject>
class CustomizeEdit:public QLineEdit
{
    Q_OBJECT
public:
    CustomizeEdit(QWidget *parent=nullptr);
    void SetMax(int max);

protected:
    void focusOutEvent(QFocusEvent* event) override{
        QLineEdit::focusOutEvent(event);
        emit sig_focus_out();
    }
    void limitTextLength(QString text){
        if(_max_length<0){
            return;
        }
        QByteArray byte=text.toUtf8();
        if(byte.length()>_max_length){
            byte=byte.left(_max_length);
            this->setText(QString::fromUtf8(byte));
        }
    }

private:

    int _max_length;

signals:
    void sig_focus_out();
};

#endif // CUSTOMIZEEDIT_H
