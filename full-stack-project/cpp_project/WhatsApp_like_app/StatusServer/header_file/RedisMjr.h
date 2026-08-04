#pragma once
#include "const.h"

class RedisMjr :public Singleton<RedisMjr> {
	friend class Singleton<RedisMjr>;
public:
    ~RedisMjr();

    // 🔑 连通大闸：所有输入参数，一律 const 焊死
    bool Connect(const std::string& host, int port, const std::string& password,int size);

    // 🎯 基础键值对：Key 只读(const)，Value 是隔空改值的肉身输出通道(非常量 &)
    bool Get(const std::string& key, std::string& value);
    bool Set(const std::string& key, const std::string& value);

    // 📡 消息队列写入：Key 和 Value 全是纯只读输入，全部加上 const
    bool LPush(const std::string& key, const std::string& value);
    bool RPush(const std::string& key, const std::string& value);

    // 📨 消息队列出队：Key 只读(const)，弹出得到的 Value 必须是输出通道(非常量 &)
    bool LPop(const std::string& key, std::string& value);
    bool RPop(const std::string& key, std::string& value);

    // 📊 哈希表定位：双重 Key 全是只读输入(const)，捞出来的 Value 隔空改值(非常量 &)
    bool HGet(const std::string& key1, const std::string& key2, std::string& value);
    bool HSet(const std::string& key1, const std::string& key2, const std::string& value);

    // 🔒 粉碎与查有无：纯输入动作，无条件 const 护航
    bool Del(const std::string& key);
    bool Exists(const std::string& key);

private:
	RedisMjr();
	std::unique_ptr<sw::redis::Redis> _redis_client;

};