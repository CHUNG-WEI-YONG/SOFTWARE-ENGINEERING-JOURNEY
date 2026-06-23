#include <iostream>
#include "Cserver.h"
#include "ConfigMgr.h"
#include <sw/redis++/redis++.h>
#include "RedisMjr.h"
#include "const.h"
#include <cassert>
#include <assert.h>


void TestRedis() {
    //连接redis 需要启动才可以进行连接
//redis默认监听端口为6387 可以再配置文件中修改
    redisContext* c = redisConnect("81.68.86.146", 6380);
    //redisContext* c = redisConnect("127.0.0.1", 6380);
    if (c->err)
    {
        printf("Connect to redisServer faile:%s\n", c->errstr);
        redisFree(c);        return;
    }
    printf("Connect to redisServer Success\n");
    std::string redis_password = "123456";
    redisReply* r = (redisReply*)redisCommand(c, "auth %s", redis_password.c_str());
    if (r->type == REDIS_REPLY_ERROR) {
        printf("Redis认证失败！\n");
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    else {
        printf("Redis认证成功！\n");
    }
    //为redis设置key
    const char* command1 = "set stest2 value1";
    //执行redis命令行
    r = (redisReply*)redisCommand(c, command1);
    //如果返回NULL则说明执行失败
    if (NULL == r)
    {
        printf("Execut command1 failure\n");
        redisFree(c);        return;
    }
    //如果执行失败则释放连接
    if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
    {
        printf("Failed to execute command[%s]\n", command1);
        printf("Failed command: %s\n", command1);

        printf("reply type = %d\n", r->type);

        if (r->str) {
            printf("reply str = %s\n", r->str);
        }

        freeReplyObject(r);
        redisFree(c);        return;
    }
    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command1);
    const char* command2 = "strlen stest1";
    r = (redisReply*)redisCommand(c, command2);
    //如果返回类型不是整形 则释放连接
    if (r->type != REDIS_REPLY_INTEGER)
    {
        printf("Failed to execute command[%s]\n", command2);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    //获取字符串长度
    int length = r->integer;
    freeReplyObject(r);
    printf("The length of 'stest1' is %d.\n", length);
    printf("Succeed to execute command[%s]\n", command2);
    //获取redis键值对信息
    const char* command3 = "get stest1";
    r = (redisReply*)redisCommand(c, command3);
    if (r->type != REDIS_REPLY_STRING)
    {
        printf("Failed to execute command[%s]\n", command3);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    printf("The value of 'stest1' is %s\n", r->str);
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command3);
    const char* command4 = "get stest2";
    r = (redisReply*)redisCommand(c, command4);
    if (r->type != REDIS_REPLY_NIL)
    {
        printf("Failed to execute command[%s]\n", command4);
        freeReplyObject(r);
        redisFree(c);        return;
    }
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command4);
    //释放连接资源
    redisFree(c);
}

void TestRedisMgr() {
    std::string value = "";

    // 🚀 1. 一枪通电连接：直接把 123456 密码作为参数喂进去
    // 底层在 reset 的万分之一秒内自动完成了三次握手与 AUTH 密码认证！
    assert(RedisMjr::GetInstance()->Connect("81.68.86.146", 6380, "123456",5));
    std::cout << "🧪 [Test 1] Connect & Auto-Auth passed!" << std::endl;

    // 🚀 2. 键值对基础读写测试 (SET & GET)
    assert(RedisMjr::GetInstance()->Set("blogwebsite", "llfc.club"));
    assert(RedisMjr::GetInstance()->Get("blogwebsite", value));
    assert(value == "llfc.club");

    // 探测一个绝对不存在的 Key，应该返回 false
    assert(RedisMjr::GetInstance()->Get("nonekey", value) == false);
    std::cout << "🧪 [Test 2] String SET & GET passed!" << std::endl;

    // 🚀 3. 哈希表双重暗号测试 (HSet & HGet)
    assert(RedisMjr::GetInstance()->HSet("bloginfo", "blogwebsite", "llfc.club"));

    // 对齐你之前重构的 HGet 接口走位：传入引用的物理地址隔空改值
    assert(RedisMjr::GetInstance()->HGet("bloginfo", "blogwebsite", value));
    assert(value == "llfc.club");
    std::cout << "🧪 [Test 3] Hash HSet & HGet passed!" << std::endl;

    // 🚀 4. 键存在性与物理粉碎测试 (ExistsKey & Del)
    assert(RedisMjr::GetInstance()->Exists("bloginfo") == true);
    assert(RedisMjr::GetInstance()->Del("bloginfo") == true);       // 第一次删除，应该成功返回 true
    assert(RedisMjr::GetInstance()->Del("bloginfo") == false);      // 第二次重复删除，因为已经被粉碎了，应该返回 false
    assert(RedisMjr::GetInstance()->Exists("bloginfo") == false); // 再次探测，应该彻底消失
    std::cout << "🧪 [Test 4] Key Exists & Del passed!" << std::endl;

    // 🚀 5. 高并发消息队列双端管道测试 (LPush / RPop / LPop)
    assert(RedisMjr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
    assert(RedisMjr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
    assert(RedisMjr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));

    // 逻辑判定：LPUSH 依次灌入 1, 2, 3，队列结构变为 [3, 2, 1]
    // RPOP 从右侧（队尾）弹出，最先出来的应该是最先灌入的 "lpushvalue1"
    assert(RedisMjr::GetInstance()->RPop("lpushkey1", value));
    assert(value == "lpushvalue1");

    assert(RedisMjr::GetInstance()->RPop("lpushkey1", value));
    assert(value == "lpushvalue2");

    // LPOP 从左侧（队头）弹出，此时队列里只剩 [3]，出来的必然是 "lpushvalue3"
    assert(RedisMjr::GetInstance()->LPop("lpushkey1", value));
    assert(value == "lpushvalue3");

    // 🛡️ 判空物理防线拦截：队列已经完全被抽干了，再次 LPOP 应该被安全拦截返回 false，绝不闪退！
    assert(RedisMjr::GetInstance()->LPop("lpushkey1", value) == false);
    assert(RedisMjr::GetInstance()->LPop("lpushkey2", value) == false);
    std::cout << "🧪 [Test 5] List LPush, LPop & RPop passed!" << std::endl;

    std::cout << "\n🎉🎉 [终极战报] 恭喜！全套现代 C++ Redis 核心功能用例全部完美合拢、全量绿灯通过！！！\n" << std::endl;
}

int main(){
    //TestRedis();
    //TestRedisMgr();
    RedisMjr::GetInstance()->Connect("127.0.0.1", 6380, "123456", 5);
	auto& gcfg=ConfigMgr::Inst();
	std::string port_number_str = gcfg["server"]["port"];
    std::cout << port_number_str<<std::endl;
	unsigned short port = atoi(port_number_str.c_str());

    try{
        unsigned short port=static_cast<unsigned short>(8080);
        net::io_context ioc{1};
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec,int signal_number){
            if(ec){
                return;
            }
            ioc.stop();
        });
        auto server=std::make_shared<Cserver>(ioc,port);
        server->start();
        ioc.run();
    }catch(std::exception const &e){
        std::cerr<<"Error "<<e.what()<<std::endl;
        return EXIT_FAILURE;
    }
}
