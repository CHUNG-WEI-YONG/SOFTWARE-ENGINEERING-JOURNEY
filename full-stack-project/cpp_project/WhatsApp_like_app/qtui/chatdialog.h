#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "customizeedit.h"
#include "global.h"
#include "statewidget.h"

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void AddUserlist();
    void ClearState(StateWidget *lb);

private:
    void ShowSearch(bool b_search=false);
    void AddLBGroup(StateWidget *lb);
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    Ui::ChatDialog *ui;
    QList<StateWidget*> _lb_list;
    QAction* _clear_action = nullptr;

public slots:
    void slot_loading_user();
    void slot_side_contact();
    void slot_side_chat();
    void slot_text_changed(const QString& str);
};


#endif // CHATDIALOG_H
