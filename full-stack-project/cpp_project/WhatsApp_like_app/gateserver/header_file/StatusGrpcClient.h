#include "const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;


class StatusConPool {
public:
	StatusConPool(const std::string& host, const std::string& port, std::size_t sz);
	std::unique_ptr<StatusService::Stub> getConn();
	void returnConn(std::unique_ptr<StatusService::Stub>);
	~StatusConPool();
	void Close();


private:
	std::string _host;
	std::string _port;
	std::size_t _pool_size;
	std::mutex _mutex;
	std::queue<std::unique_ptr<StatusService::Stub>> _connections;
	atomic<bool> _b_stop;
	std::condition_variable _cv;
};



class StatusGrpcClient :public Singleton<StatusGrpcClient> {
	friend Singleton<StatusGrpcClient>;
public:
	~StatusGrpcClient() {};
	GetChatServerRsp GetChatServer(int uid);

private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> _pool;

};