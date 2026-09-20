#include "RedisMjr.h"
#include "ConfigMgr.h"
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <thread>
#include <string>
#include <random>
#include <hiredis/hiredis.h>


std::string generateUUID() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return to_string(uuid);
}

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
    auto& gCfgMgr = ConfigMgr::Inst();
    auto host = gCfgMgr["RedisServer"]["Host"];
    auto port = gCfgMgr["RedisServer"]["Port"];
    auto pwd = gCfgMgr["RedisServer"]["Passwd"];
    _pool.reset(new RedisConPool(10, host.c_str(), atoi(port.c_str()), pwd.c_str()));
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

bool RedisMjr::HDel(const std::string& key1, const std::string& key2) {
    auto conn = _pool->getConnection();
    if (conn == nullptr) {
        return false;
    }
    Defer defer([&conn, this]() {
        _pool->returnConnection(conn);
        });
    redisReply* reply = (redisReply*)redisCommand(conn, "HDEL %s %s", key1.c_str(), key2.c_str());
    if (reply == nullptr) {
        std::cout << "HDEL command wrong" << std::endl;
        return false;
    }
    bool success = false;
    if (reply->type == REDIS_REPLY_INTEGER) {
        success = reply->integer > 0;
    }
    freeReplyObject(reply);
    return success;
}


std::string RedisMjr::acquireLock(const std::string& lock_name, int acquireTime, int locktime)
{
    auto conn = _pool->getConnection();
    if (conn == nullptr) {
        return "";

    }

    Defer defer([this, &conn] {
        _pool->returnConnection(std::move(conn));
        });

    return DistLock::Inst().acquired_lock(conn, lock_name, acquireTime, locktime);
}

bool RedisMjr::releaseLock(const std::string& lock_name, const std::string& identifier)
{
    
    if (identifier.empty()) {
        return true;
    }

    auto conn = _pool->getConnection();
    if (conn == nullptr) {
        return false;

    }
    Defer defer([this, &conn] {
        _pool->returnConnection(std::move(conn));
        });

    return DistLock::Inst().release_lock(conn, lock_name, identifier);
}

void RedisMjr::DelCount(std::string server_name) {
    auto key = LOCK_COUNT;
    auto identifier = RedisMjr::GetInstance()->acquireLock(key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);

    Defer defer([&key, &identifier]() {
        RedisMjr::GetInstance()->releaseLock(key, identifier);
        });

    RedisMjr::GetInstance()->HDel(LOGIN_COUNT, server_name);

}

DistLock& DistLock::Inst()
{
    static DistLock lock;
    return lock;
    // TODO: insert return statement here
}

DistLock::~DistLock() {

}

std::string DistLock::acquired_lock(redisContext* context, const std::string& lockname, int lock_timeout, int acquire_timeout)
{
    if (!context) {
        return "";
    }
    std::string identifier = generateUUID();
    std::string key =LOCK_PREFIX + lockname;
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(acquire_timeout);
    
    while (std::chrono::steady_clock::now() < endTime) {
        redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s NX EX %d", key.c_str(), identifier.c_str(), lock_timeout);
        if (reply!=nullptr) {
            bool success = (reply->type == REDIS_REPLY_STATUS) && (std::string(reply->str) == "OK");
            freeReplyObject(reply);
            if(success) return identifier;
        }
        else {
            if (context->err) {
                return "";
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
    return "";
}

bool DistLock::release_lock(redisContext* context, const std::string& lockname, const string& identifier)
{
    if (!context || context->err || identifier.empty())return false;
    auto key = LOCK_PREFIX + lockname;
    const char* luaScript = R"(
        if redis.call('get', KEYS[1]) == ARGV[1] then
            return redis.call('del', KEYS[1])
        else
            return 0
        end
    )";
    redisReply* reply = (redisReply*)redisCommand(context, "EVAL %s 1 %s %s",
        luaScript, key.c_str(), identifier.c_str());

    bool success = false;
    if (reply != nullptr) {
        if (reply->type == REDIS_REPLY_STATUS && reply->integer==1) {
            success = true;
        }
        freeReplyObject(reply);
        
    }


    return success;
}


