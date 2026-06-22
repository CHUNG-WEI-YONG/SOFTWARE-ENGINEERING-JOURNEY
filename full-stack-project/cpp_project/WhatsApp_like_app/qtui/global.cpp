#include "global.h"
#include <iostream>
#include <regex>
#include <string>

QString gate_url_prefix="";

std::function<void(QWidget *)>repolish = [](QWidget *w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

bool ValidatePasswordStyle(const std::string& password, std::string& error_msg) {
    // 1. 物理长度硬卡位
    if (password.length() < 8 || password.length() > 20) {
        error_msg = "Password length must be between 8 and 20 characters.";
        return false;
    }

    // 2. 强密码正则轰炸
    std::regex password_regex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,20}$");

    if (!std::regex_match(password, password_regex)) {
        error_msg = "Password must contain uppercase, lowercase, number, and special character.";
        return false;
    }

    return true;
}