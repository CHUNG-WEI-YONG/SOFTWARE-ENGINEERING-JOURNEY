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

std::function<QString(QString)> xorString=[](QString input){
//     QString result=input;
//     int length=input.length();
//     length=length%255;
//     for(int i=0;i<length;i++){
//         result[i]=QChar(static_cast<ushort>(input[i].unicode()^static_cast<ushort>(length)));
//     }
//     return result;
// };


// auto xorString = [](const QString& input) -> QString {
    if (input.isEmpty()) return input;

    // 🎯 1. 物理转换为纯字节流，消灭 UTF-16 乱码隐患
    QByteArray bytes = input.toUtf8();
    int len = bytes.length();

    // 🎯 2. 安全发号：防止 len % 255 == 0 导致加密失效，给个保底密钥暗号（例如 0x5A）
    char key = (len % 255 == 0) ? 0x5A : static_cast<char>(len % 255);

    // 🎯 3. 黄金走位：直接利用 QByteArray 内部的指针进行原地异或，速度快如闪电
    char* data = bytes.data();
    for (int i = 0; i < len; ++i) {
        data[i] ^= key; // 纯物理位运算，没有多余的写时复制开销！
    }

    // 🎯 4. 安全打包回执
    return QString::fromUtf8(bytes);
};