#include "customizeedit.h"

CustomizeEdit::CustomizeEdit(QWidget *parent):QLineEdit(parent),_max_length(0) {
    connect(this,&QLineEdit::textChanged,this,&CustomizeEdit::limitTextLength);

}

void CustomizeEdit::SetMax(int max){
    _max_length=max;
}
