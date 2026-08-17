#include "authfriend.h"
#include "ui_authfriend.h"
#include <QScrollBar>
#include <QJsonObject>
#include <QJsonDocument>
#include "usermgr.h"
#include "tcpmgr.h"

AuthFriend::AuthFriend(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthFriend)
    , _label_point(2, 6)
{
    ui->setupUi(this); // 修复：仅保留一次 setupUi

    // 隐藏对话框标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("AuthFriend");
    this->setModal(true);

    ui->name_edit->setPlaceholderText(tr("恋恋风辰"));
    ui->name_edit->SetMax(20);
    ui->lb_edit->setPlaceholderText("搜索、添加标签");
    ui->nick_edit->setPlaceholderText("燃烧的胸毛");
    ui->nick_edit->SetMax(20);

    ui->lb_edit->SetMax(21);
    ui->lb_edit->setFixedHeight(20);
    ui->lb_edit->setMaxLength(10);
    ui->input_wid->hide();

    _tip_cur_point = QPoint(5, 5);

    _tip_data = { "同学","家人","菜鸟教程","C++ Primer","Rust 程序设计",
                 "父与子学Python","nodejs开发指南","go 语言开发指南",
                 "游戏伙伴","金融投资","微信读书","拼多多拼友" };

    connect(ui->more_lb, &ClickedOnceLabel::Clicked, this, &AuthFriend::ShowMoreLabel);
    InitTipLbs();

    // 链接输入标签回车及输入事件
    connect(ui->lb_edit, &CustomizeEdit::returnPressed, this, &AuthFriend::SlotLabelEnter);
    connect(ui->lb_edit, &CustomizeEdit::textChanged, this, &AuthFriend::SlotLabelTextChange);
    connect(ui->lb_edit, &CustomizeEdit::editingFinished, this, &AuthFriend::SlotLabelEditFinished);
    connect(ui->tip_lb, &ClickedOnceLabel::Clicked, this, &AuthFriend::SlotAddFirendLabelByClickTip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);

    // 连接确认和取消按钮的槽函数
    connect(ui->cancel_btn, &QPushButton::clicked, this, &AuthFriend::SlotApplyCancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &AuthFriend::SlotApplySure);
}

AuthFriend::~AuthFriend()
{
    qDebug() << "Authen friend destruct";
    delete ui;
}

void AuthFriend::InitTipLbs()
{
    int lines = 1;
    for (int i = 0; i < _tip_data.size(); i++) {
        auto* lb = new ClickedLabel(ui->lb_list);
        lb->setState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickedLabel::clicked, this, &AuthFriend::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font());
        int textWidth = fontMetrics.horizontalAdvance(lb->text());
        int textHeight = fontMetrics.height();

        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {
            lines++;
            if (lines > 2) {
                delete lb;
                return;
            }

            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }

        auto next_point = _tip_cur_point;
        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
        _tip_cur_point = next_point;
    }
}

void AuthFriend::AddTipLbs(ClickedLabel* lb, QPoint cur_point, QPoint& next_point, int text_width, int text_height)
{
    Q_UNUSED(text_height);
    lb->move(cur_point);
    lb->show();
    _add_labels.insert(lb->text(), lb);
    _add_label_keys.push_back(lb->text());
    next_point.setX(lb->pos().x() + text_width + 15);
    next_point.setY(lb->pos().y());
}

bool AuthFriend::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->scrollArea && event->type() == QEvent::Enter) {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave) {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}

void AuthFriend::SetApplyInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    qDebug()<<"Auth friend page get apply info that uid is "<<apply_info->_uid;
    _apply_info = apply_info;
    ui->name_edit->setPlaceholderText(apply_info->_name);
}

void AuthFriend::ShowMoreLabel()
{
    qDebug() << "receive more label clicked";
    ui->more_lb_wid->hide();

    ui->lb_list->setFixedWidth(325);
    _tip_cur_point = QPoint(5, 5);
    auto next_point = _tip_cur_point;
    int textWidth = 0;
    int textHeight = 0;

    // 重排现有的 label
    for (auto & added_key : _add_label_keys) {
        auto added_lb = _add_labels[added_key];
        QFontMetrics fontMetrics(added_lb->font());
        textWidth = fontMetrics.horizontalAdvance(added_lb->text());
        textHeight = fontMetrics.height();

        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }
        added_lb->move(_tip_cur_point);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(_tip_cur_point.y());
        _tip_cur_point = next_point;
    }

    // 添加未添加的
    for (int i = 0; i < _tip_data.size(); i++) {
        auto iter = _add_labels.find(_tip_data[i]);
        if (iter != _add_labels.end()) {
            continue;
        }

        auto* lb = new ClickedLabel(ui->lb_list);
        lb->setState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(_tip_data[i]);
        connect(lb, &ClickedLabel::clicked, this, &AuthFriend::SlotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font());
        textWidth = fontMetrics.horizontalAdvance(lb->text());
        textHeight = fontMetrics.height();

        if (_tip_cur_point.x() + textWidth + tip_offset > ui->lb_list->width()) {
            _tip_cur_point.setX(tip_offset);
            _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
        }

        next_point = _tip_cur_point;
        AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
        _tip_cur_point = next_point;
    }

    int old_list_height = ui->lb_list->height();
    int final_more_height = next_point.y() + textHeight + tip_offset;
    ui->lb_list->setFixedHeight(final_more_height);

    int diff_height = final_more_height - old_list_height;
    ui->scrollContent->setFixedHeight(ui->scrollContent->height() + diff_height);

    ui->scrollContent->updateGeometry();
    if (ui->scrollContent->layout()) {
        ui->scrollContent->layout()->activate();
    }

    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->maximum());
}

void AuthFriend::resetLabels()
{
    _label_point = QPoint(2, 6);
    auto max_width = ui->grid_wid->width();
    auto label_height = 0;

    for (auto iter = _friend_labels.begin(); iter != _friend_labels.end(); iter++) {
        if (_label_point.x() + iter.value()->width() > max_width) {
            _label_point.setY(_label_point.y() + iter.value()->height() + 6);
            _label_point.setX(2);
        }

        iter.value()->move(_label_point);
        iter.value()->show();

        _label_point.setX(_label_point.x() + iter.value()->width() + 2);
        label_height = iter.value()->height();
    }

    if (_friend_labels.isEmpty()) {
        ui->lb_edit->move(_label_point);
        ui->grid_wid->setFixedHeight(32);
        ui->lb_group_wid->setFixedHeight(42);
    } else {
        if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_wid->width()) {
            ui->lb_edit->move(2, _label_point.y() + label_height + 6);
        } else {
            ui->lb_edit->move(_label_point);
        }
        int target_height = _label_point.y() + label_height + 6;
        ui->grid_wid->setFixedHeight(target_height);
        ui->lb_group_wid->setFixedHeight(target_height + 10);
    }

    ui->scrollContent->updateGeometry();
    if (ui->scrollContent->layout()) {
        ui->scrollContent->layout()->activate();
    }
}

void AuthFriend::addLabel(QString name)
{
    if (_friend_labels.find(name) != _friend_labels.end()) {
        ui->lb_edit->clear();
        ui->lb_edit->setFocus();
        return;
    }

    auto tmplabel = new FriendLabel(ui->grid_wid);
    tmplabel->SetText(name);
    tmplabel->setObjectName("FriendLabel");

    auto max_width = ui->grid_wid->width();
    if (_label_point.x() + tmplabel->width() > max_width) {
        _label_point.setY(_label_point.y() + tmplabel->height() + 6);
        _label_point.setX(2);
    }

    tmplabel->move(_label_point);
    tmplabel->show();
    _friend_labels[tmplabel->text()] = tmplabel;
    _friend_label_keys.push_back(tmplabel->text());

    connect(tmplabel, &FriendLabel::sig_close, this, &AuthFriend::SlotRemoveFriendLabel);

    _label_point.setX(_label_point.x() + tmplabel->width() + 2);

    if (_label_point.x() + MIN_APPLY_LABEL_ED_LEN > ui->grid_wid->width()) {
        _label_point.setY(_label_point.y() + tmplabel->height() + 6);
        _label_point.setX(2);
        ui->lb_edit->move(_label_point);
    } else {
        ui->lb_edit->move(_label_point);
    }

    ui->lb_edit->clear();
    ui->lb_edit->setFocus();

    int target_grid_height = _label_point.y() + tmplabel->height() + 6;
    if (ui->grid_wid->height() < target_grid_height) {
        ui->grid_wid->setFixedHeight(target_grid_height);
        ui->lb_group_wid->setFixedHeight(target_grid_height + 10);
    }

    ui->scrollContent->updateGeometry();
    if (ui->scrollContent->layout()) {
        ui->scrollContent->layout()->activate();
    }
}

void AuthFriend::SlotLabelEnter()
{
    if (ui->lb_edit->text().isEmpty()) {
        return;
    }

    auto text = ui->lb_edit->text();
    addLabel(text);
    ui->input_wid->hide();

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
    }

    ui->lb_edit->setFocus();
}

void AuthFriend::SlotRemoveFriendLabel(QString name)
{
    qDebug() << "receive close signal";

    auto find_iter = _friend_labels.find(name);
    if (find_iter == _friend_labels.end()) {
        return;
    }

    auto find_key = std::find(_friend_label_keys.begin(), _friend_label_keys.end(), name);
    if (find_key != _friend_label_keys.end()) {
        _friend_label_keys.erase(find_key);
    }

    delete find_iter.value();
    _friend_labels.erase(find_iter);

    resetLabels();

    auto find_add = _add_labels.find(name);
    if (find_add != _add_labels.end()) {
        find_add.value()->ResetNormalState();
    }
}

void AuthFriend::SlotChangeFriendLabelByTip(QString lbtext, ClickLbState state)
{
    auto find_iter = _add_labels.find(lbtext);
    if (find_iter == _add_labels.end()) {
        return;
    }

    if (state == ClickLbState::Selected) {
        addLabel(lbtext);
    } else if (state == ClickLbState::Normal) {
        SlotRemoveFriendLabel(lbtext);
    }
}

void AuthFriend::SlotLabelTextChange(const QString& text)
{
    if (text.isEmpty()) {
        ui->tip_lb->setText("");
        ui->input_wid->hide();
        return;
    }

    auto iter = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (iter == _tip_data.end()) {
        ui->tip_lb->setText(add_prefix + text);
    } else {
        ui->tip_lb->setText(text);
    }
    ui->input_wid->show();
}

void AuthFriend::SlotLabelEditFinished()
{
    ui->input_wid->hide();
}

void AuthFriend::SlotAddFirendLabelByClickTip(QString text)
{
    int index = text.indexOf(add_prefix);
    if (index != -1) {
        text = text.mid(index + add_prefix.length());
    }
    addLabel(text);

    auto find_it = std::find(_tip_data.begin(), _tip_data.end(), text);
    if (find_it == _tip_data.end()) {
        _tip_data.push_back(text);
    }

    auto find_add = _add_labels.find(text);
    if (find_add != _add_labels.end()) {
        find_add.value()->SetCurState(ClickLbState::Selected);
        return;
    }

    auto* lb = new ClickedLabel(ui->lb_list);
    lb->setState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this, &AuthFriend::SlotChangeFriendLabelByTip);

    QFontMetrics fontMetrics(lb->font());
    int textWidth = fontMetrics.horizontalAdvance(lb->text());
    int textHeight = fontMetrics.height();

    if (_tip_cur_point.x() + textWidth + tip_offset + 3 > ui->lb_list->width()) {
        _tip_cur_point.setX(5);
        _tip_cur_point.setY(_tip_cur_point.y() + textHeight + 15);
    }

    auto next_point = _tip_cur_point;
    AddTipLbs(lb, _tip_cur_point, next_point, textWidth, textHeight);
    _tip_cur_point = next_point;

    int old_list_height = ui->lb_list->height();
    int final_click_height = next_point.y() + textHeight + tip_offset;
    ui->lb_list->setFixedHeight(final_click_height);

    int diff_height = final_click_height - old_list_height;
    ui->scrollContent->setFixedHeight(ui->scrollContent->height() + diff_height);

    lb->SetCurState(ClickLbState::Selected);

    ui->scrollContent->updateGeometry();
    if (ui->scrollContent->layout()) {
        ui->scrollContent->layout()->activate();
    }
}

void AuthFriend::SlotApplySure()
{
    qDebug() << "Slot Apply Sure called";
    QJsonObject jsonObj;
    auto uid = UserMgr::getInstance()->GetUid();
    jsonObj["fromuid"] = uid;
    jsonObj["touid"] = _apply_info->_uid;

    QString back_name = ui->nick_edit->text().isEmpty() ?
                            ui->nick_edit->placeholderText() :
                            ui->nick_edit->text();
    jsonObj["nickname"] = back_name;

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    qDebug()<<"Auth friend sending "<<_apply_info->_uid;
    emit TcpMgr::getInstance()->sig_send_data(ReqId::ID_AUTH_FRIEND_REQ, jsonData);
    this->hide();
    deleteLater();
}

void AuthFriend::SlotApplyCancel()
{
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}
