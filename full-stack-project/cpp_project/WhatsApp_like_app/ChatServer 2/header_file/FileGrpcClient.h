#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <mutex>
#include <queue>
#include <condition_variable> 
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::GetChatServerRsp;
using message::LoginRsp;
using message::LoginReq;
using message::ChatService;

using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using message::TextChatData;

using message::ApplyUploadReq;
using message::ApplyUploadRsp;
using message::FileService;

class FileConnPool {
public:
    // 默认参数放在最后，传参使用 const 引用避免拷贝
    FileConnPool(const std::string& host, const std::string& port, int sz = 5)
        : _host(host), _port(port), _sz(sz), _b_stop(false)
    {
        for (int i = 0; i < sz; i++) {
            std::shared_ptr<Channel> channel = grpc::CreateChannel(
                host + ":" + port,
                grpc::InsecureChannelCredentials()
            );
            _queue.push(FileService::NewStub(channel));
        }
    }

    std::unique_ptr<FileService::Stub> getConn() {
        std::unique_lock<std::mutex> lock(_mtx);

        // 关键修复：队列非空 (!_queue.empty()) 或者连接池停止时才唤醒
        _cv.wait(lock, [this]() {
            return _b_stop.load() || !_queue.empty();
            });

        // 如果池子已停止且队列空了，安全返回 nullptr
        if (_b_stop.load() && _queue.empty()) {
            return nullptr;
        }

        auto con = std::move(_queue.front());
        _queue.pop();
        return con;
    }

    void returnConn(std::unique_ptr<FileService::Stub> conn) {
        if (!conn) {
            return;
        }

        std::lock_guard<std::mutex> lock(_mtx);
        if (_b_stop.load()) {
            return;
        }
        _queue.push(std::move(conn));
        _cv.notify_one();
    }

    ~FileConnPool() {
        Stop();
    }

    void Stop() {
        if (_b_stop.exchange(true)) {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(_mtx);
            // std::queue 没有 clear()，使用 swap 优雅清空
            std::queue<std::unique_ptr<FileService::Stub>> empty;
            std::swap(_queue, empty);
        }

        _cv.notify_all();
    }

private:
    std::mutex _mtx;
    std::condition_variable _cv;
    std::queue<std::unique_ptr<FileService::Stub>> _queue;
    int _sz;
    std::string _host;
    std::string _port;
    std::atomic<bool> _b_stop;
};

class FileGrpcClient:public Singleton<FileGrpcClient> {
    friend class Singleton<FileGrpcClient>;
private:
    FileGrpcClient();
    std::unique_ptr<FileConnPool> _pool;

public:
    ApplyUploadRsp ApplyUploadTicket(int from_uid ,int to_uid,string file_name,int64_t file_size,string md5);

};