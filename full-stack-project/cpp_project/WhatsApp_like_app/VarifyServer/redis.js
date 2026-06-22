const config_module = require('./config');
const redis = require("ioredis");

// 🚀 1. 建立全局唯一、自带高性能连接池的 Redis 触手
const RedisCli = new redis({
    host: config_module.redis_host,
    port: config_module.redis_port,
    password: config_module.redis_passwd
});

/** * 从 Redis 缓存中获取指定键的值
 * @param {string} key
 * @returns {Promise<string|null>}
 */
async function RedisGet(key) {
    try {
        const value = await RedisCli.get(key);    
        if (value == null) {
            console.log(`📡 [Redis Get] Key '${key}' does not exist (Expired or not set).`);
            return null;
        }            
        console.log(`🟢 [Redis Get] Key '${key}' successfully retrieved ->`, value);
        return value;
    }
    catch (error) {
        console.error(`🔴 [Redis Get Exception] Failed to query key '${key}':`, error);
        return null;
    }
}

/** * 检查指定的 Key 是否在 Redis 中物理存在
 * @param {string} key
 * @returns {Promise<number>} 返回 1 代表存在，0 代表不存在
 */
async function QueryRedis(key) {
    try {
        const result = await RedisCli.exists(key);    
        if (result == 0) {
            console.log(`📡 [Redis Query] Key '${key}' does not exist (Expired or not set).`);
            return 0;
        }            
        // 🎯 核心修复 1：将原本找不到定义肉身的 value 改为对齐打印 result
        console.log(`🟢 [Redis Query] Key '${key}' check result ->`, result);
        return result;
    }
    catch (error) {
        console.error(`🔴 [Redis Query Exception] Failed to check key '${key}':`, error);
        return 0;
    }
}

/**
 * 写入一个自带全自动物理蒸发（TTL）的键值对
 * @param {string} key
 * @param {string} value
 * @param {number} expired 过期时间（单位：秒）
 * @returns {Promise<boolean>}
 */
async function SetRedisExpire(key, value, expired) {
    try {
        // 🚀 🏆 核心修复 2：大厂级终极原子合并流！
        // 彻底抛弃危险的两步法，用 "EX" 让 Redis 在底层百万分之一秒内一次性搞定 SET + EXPIRE！
        await RedisCli.set(key, value, "EX", expired);
        
        console.log(`🟢 [Redis SetExpire] Key '${key}' successfully set with TTL: ${expired}s`);
        return true;
    }
    catch (error) {
        console.error(`🔴 [Redis SetExpire Exception] Failed to set expired key '${key}':`, error);
        return false;
    }
}

/**
 * 优雅切断连接池长连接，防止进程挂起
 */
function quit() {
    RedisCli.quit();
    console.log("🔒 [Redis Pool] Connection pool gracefully shutdown.");
}

// 📦 统一物理出口
module.exports = { SetRedisExpire, RedisGet, QueryRedis, quit };