#include "RedisMjr.h"

bool RedisMjr::Connect(const std::string& host, int port, const std::string& password,int size) {
    try {
        sw::redis::ConnectionOptions opts;
        opts.host = host;
        opts.port = port;
        opts.password = password;

		sw::redis::ConnectionPoolOptions pool_opts;
        pool_opts.size = size;

        this->_redis_client.reset(new sw::redis::Redis(opts,pool_opts));
        this->_redis_client->ping();
        std::cout << "Connected to the Redis server " << host << ":" << port << std::endl;
        return true;
    }
    catch (const sw::redis::Error& err) {
        std::cout << "Failed to connect to the Redis server " << host << ":" << port << ". Error: " << err.what() << std::endl;
        this->_redis_client.reset(nullptr);
        return false;
    }
}

bool RedisMjr::Get(const std::string& key, std::string& value) {
    try {
        auto reply = this->_redis_client->get(key);
        if (!reply) {
            std::cout << "Key '" << key << "' does not exist in Redis." << std::endl;
            return false;
        }
        value = *reply; // 🎯 顺着传入的非常量引用地址，隔空改值
        std::cout << "Value for key '" << key << "': " << value << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis GET command failed: " << e.what() << std::endl;
        return false;
    }
}

bool RedisMjr::Set(const std::string& key, const std::string& value) {
    try {
        this->_redis_client->set(key, value);
        std::cout << "Setting correct [Key: " << key << ", Value: " << value << "]" << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis SET command failed: " << e.what() << std::endl;
        return false;
    }
}

bool RedisMjr::LPush(const std::string& key, const std::string& value) {
    try {
        this->_redis_client->lpush(key, value);
        std::cout << "LPush correct [Key: " << key << ", Value: " << value << "]" << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis LPush command failed: " << e.what() << std::endl;
        return false;
    }
}

bool RedisMjr::RPush(const std::string& key, const std::string& value) {
    try {
        this->_redis_client->rpush(key, value);
        std::cout << "RPush correct [Key: " << key << ", Value: " << value << "]" << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis RPush command failed: " << e.what() << std::endl;
        return false;
    }
}

bool RedisMjr::LPop(const std::string& key, std::string& value) {
    try {
        auto reply = this->_redis_client->lpop(key);
        if (!reply) {
            std::cout << "Key '" << key << "' does not exist in Redis List (Empty)." << std::endl;
            return false;
        }
        value = *reply; // 🎯 输出型参数传回
        std::cout << "LPop correct [Key: " << key << ", Popped Value: " << value << "]" << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis LPop command failed: " << e.what() << std::endl;
        return false;
    }
}

bool RedisMjr::RPop(const std::string& key, std::string& value) {
    try {
        auto reply = this->_redis_client->rpop(key);
        if (!reply) {
            std::cout << "Key '" << key << "' does not exist in Redis List (Empty)." << std::endl;
            return false;
        }
        value = *reply; // 🎯 输出型参数传回
        std::cout << "RPop correct [Key: " << key << ", Popped Value: " << value << "]" << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis RPop command failed: " << e.what() << std::endl;
        return false;
    }
}

bool RedisMjr::HGet(const std::string& key1, const std::string& key2, std::string& value) {
    try {
        auto reply = this->_redis_client->hget(key1, key2);
        if (!reply) {
            std::cout << "Hash Key '" << key1 << " " << key2 << "' does not exist in Redis." << std::endl;
            return false;
        }
        value = *reply; // 🎯 输出型参数传回
        std::cout << "HGet correct [Key: " << key1 << ", Field: " << key2 << "] -> Value: " << value << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis HGet command failed: " << e.what() << std::endl;
        return false;
    }
}

bool RedisMjr::HSet(const std::string& key1, const std::string& key2, const std::string& value) {
    try {
        this->_redis_client->hset(key1, key2, value);
        std::cout << "HSet correct [Key: " << key1 << ", Field: " << key2 << "] value is " << value << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis HSet command failed: " << e.what() << std::endl;
        return false;
    }
}

bool RedisMjr::Del(const std::string& key) {
    try {
        auto reply = this->_redis_client->del(key);
        if (reply == 0) {
            std::cout << "Key '" << key << "' does not exist in Redis." << std::endl;
            return false;
        }
        std::cout << "Delete correct [Key: " << key << "]" << std::endl;
        return true;
    }
    catch (const sw::redis::Error& e) {
        std::cout << "Redis DEL command failed: " << e.what() << std::endl;
        return false;
    }
}

RedisMjr::~RedisMjr() {
    std::cout << "🟢 [RAII 断电] RedisMjr 析构，底层 TCP 套接字池已全自动安全回收！" << std::endl;
}

// 🚀 复活构造函数：把单例初期的初始化逻辑安排上
RedisMjr::RedisMjr() : _redis_client(nullptr) {
    // 可以在这里做一些基础的成员变量初始化
}

bool RedisMjr::Exists(const std::string& key) {
	try {
		bool exists = this->_redis_client->exists(key);
		std::cout << "Exists check for key '" << key << "': " << (exists ? "true" : "false") << std::endl;
		return exists;
	}
	catch (const sw::redis::Error& e) {
		std::cout << "Redis EXISTS command failed: " << e.what() << std::endl;
		return false;
	}
}