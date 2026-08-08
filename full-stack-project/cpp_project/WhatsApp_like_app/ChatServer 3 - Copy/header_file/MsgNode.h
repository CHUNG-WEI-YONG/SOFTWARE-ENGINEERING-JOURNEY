#pragma once
#include <string>
#include "const.h"
#include <iostream>
#include <boost/asio.hpp>
#include <cstring>


class LogicSystem;

class MsgNode {
    friend class CSession;

protected:
    short _total_len;
    short curr_len;
    char* _data;

public:
    MsgNode(short len)
        : _total_len(len), curr_len(0) {
        _data = new char[_total_len + 1]();
        _data[_total_len] = '\0';
    }

    virtual ~MsgNode() {
        delete[] _data;
        _data = nullptr;
    }

    MsgNode(const MsgNode&) = delete;
    MsgNode& operator=(const MsgNode&) = delete;

    void Clear() {
        curr_len = 0;
        std::memset(_data, 0, _total_len);
    }
};


class RecvNode :public MsgNode {
	friend class LogicSystem;
public:
	RecvNode(int len, short msg_id);
	short _msg_id;
};

class SendNode :public MsgNode {
	friend class LogicSystem;
public:
	SendNode(const char* msg, short msg_len, short msg_id);
	short _msg_id;
};