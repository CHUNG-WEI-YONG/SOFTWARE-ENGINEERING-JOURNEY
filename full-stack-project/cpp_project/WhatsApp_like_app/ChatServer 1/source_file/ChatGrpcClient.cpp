#include "ChatGrpcClient.h"
#include "ConfigMgr.h"
#include "RedisMjr.h"
#include "MySqlMgr.h"

ChatGrpcClient::ChatGrpcClient() {
    auto& config = ConfigMgr::Inst();
    auto serverlist = config["PeerServer"]["Servers"];

    std::vector<std::string> words;
    std::string word;

    std::stringstream ss(serverlist);
    while (std::getline(ss, word, ',')) {
        words.push_back(word);
    }
    for (auto& w : words) {
        SectionInfo section = config[w];
        std::string name = section["Name"];
        std::string host = section["Host"];
        std::string port = section["RPCPort"];

        if (name.empty()) {
            continue;
        }

        std::cout << "Connect correctly to " << w << " host :" << host << " rpc port: " << port << "\n";

        // ✅ 同时存入 "Host:Port" 和 "Host"，提高查找容错率
        std::string ip_port = host + ":" + port;
        _ip_to_name[ip_port] = name;

        // 初始化连接池
        _pools[name] = std::make_unique<ChatConPool>(5, host, port);
    }
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_name, const AddFriendReq& req) {
    AddFriendRsp rsp;
    rsp.set_applyuid(req.applyuid());
    rsp.set_touid(req.touid());
    std::cout << "Notify success run" << std::endl;
    // 3. 获取对应的连接池
    auto iter = _pools.find(server_name);
    if (iter == _pools.end()) {
        std::cout << "Cannot find connection pool for server_name: " << server_name << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    auto& pool = iter->second;
    auto conn = pool->GetConn();
    if (!conn) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    // 4. 析构时归还连接
    Defer rtConn([&conn, &pool]() {
        pool->returnConn(std::move(conn));
        });

    // 5. 发送 RPC 请求
    ClientContext context;
    Status status = conn->NotifyAddFriend(&context, req, &rsp);

    if (!status.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    rsp.set_error(ErrorCodes::Success);
    return rsp;
}
AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_name, const AuthFriendReq& req) {
	AuthFriendRsp rsp;
    int from_uid = req.fromuid();
    int to_uid = req.touid();
    rsp.set_fromuid(from_uid);
    rsp.set_touid(to_uid);
    rsp.set_error(ErrorCodes::Success);

    auto iter = _pools.find(server_name);
    if (iter == _pools.end()) {
        cout << "Error Grpc not find " << server_name;
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }
    auto& pool = iter->second;
    auto conn = pool->GetConn();
    if (!conn) {
        std::cout << "Error: Failed to get connection from pool: " << server_name << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed); // 修复：连接获取失败处理
        return rsp;
    }

    Defer defer([&pool, &conn]() {
        pool->returnConn(std::move(conn));
        });

    ClientContext context;
    Status status = conn->NotifyAuthFriend(&context, req, &rsp);
    if (!status.ok()) {
        std::cerr << "gRPC NotifyAuthFriend to " << server_name
            << " failed: " << status.error_message() << std::endl;
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }
    return rsp;

}
bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {
    std::string info_str = "";
    bool b_base = RedisMjr::GetInstance()->Get(base_key, info_str);
    if (b_base) {
        Json::Reader reader;
        Json::Value root;
        reader.parse(info_str, root);
        userinfo->uid = root["uid"].asInt();
        userinfo->name = root["name"].asString();
        userinfo->passwd = root["pwd"].asString();
        userinfo->email = root["email"].asString();
        userinfo->nick = root["nick"].asString();
        userinfo->desc = root["desc"].asString();
        userinfo->sex = root["sex"].asInt();
        userinfo->icon = root["icon"].asString();
        std::cout << "user login uid is  " << userinfo->uid << " name  is "
            << userinfo->name << " pwd is " << userinfo->passwd << " email is " << userinfo->email << endl;
    }
    else {
        //redis中没有则查询mysql
        //查询数据库
        std::shared_ptr<UserInfo> user_info = nullptr;
        user_info = MysqlMgr::GetInstance()->GetUser(uid);
        if (user_info == nullptr) {
            return false;
        }

        userinfo = user_info;

        //将数据库内容写入redis缓存
        Json::Value redis_root;
        redis_root["uid"] = uid;
        redis_root["passwd"] = userinfo->passwd;
        redis_root["name"] = userinfo->name;
        redis_root["email"] = userinfo->email;
        redis_root["nick"] = userinfo->nick;
        redis_root["desc"] = userinfo->desc;
        redis_root["sex"] = userinfo->sex;
        redis_root["icon"] = userinfo->icon;
        RedisMjr::GetInstance()->Set(base_key, redis_root.toStyledString());
    }
}


TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue) {
	TextChatMsgRsp rsp;
    int from_uid = req.fromuid();
    int to_uid = req.touid();

    //TextChatData* new_msg=rsp.add_textmsgs();
    //for (const auto& text_data : req.textmsgs()) {
    //    TextChatData* new_msg = rsp.add_textmsgs();
    //    new_msg->set_unique_id(text_data.unique_id());
    //    new_msg->set_msgcontent(text_data.msgcontent());
    //}

    auto iter = _pools.find(server_ip);
    if (iter == _pools.end()) {
        cout << "Grpc Cannot find server_ip: " << server_ip;
        return rsp;
    }
    auto& pool = iter->second;
    auto stub = pool->GetConn();
    Defer defer([&pool, &stub]() {
        pool->returnConn(std::move(stub));
        });

    ClientContext context;
    Status result=stub->NotifyTextChatMsg(&context, req, &rsp);

    if (!result.ok()) {
        rsp.set_error(ErrorCodes::RPCFailed);
        return rsp;
    }

    return rsp;

}