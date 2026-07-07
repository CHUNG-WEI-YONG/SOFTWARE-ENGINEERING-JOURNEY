#include "StatusGrpcClient.h"
#include "ConfigMgr.h"

StatusConPool::StatusConPool(const std::string& host, const std::string& port, std::size_t sz):_host(host),_port(port),_pool_size(sz) {
	for (int i = 0;i < _pool_size;i++) {
		auto channel = grpc::CreateChannel(_host + ":" + _port,grpc::InsecureChannelCredentials());
		auto stub = StatusService::NewStub(channel);
		_connections.push(std::move(stub));
	}

}

StatusConPool::~StatusConPool() {
	std::lock_guard<std::mutex> lock(_mutex);
	Close();
	while (!_connections.empty()) {
		_connections.pop();
	}
}

std::unique_ptr<StatusService::Stub> StatusConPool:: getConn() {
	std::unique_lock<std::mutex> lock(_mutex);
	_cv.wait(lock, [this] {
		if (_b_stop) {
			return true;
		}
		return !_connections.empty();
		});
	//如果停止则直接返回空指针
	if (_b_stop) {
		return  nullptr;
	}
	auto context = std::move(_connections.front());
	_connections.pop();
	return context;
}

void StatusConPool::returnConn(std::unique_ptr<StatusService::Stub> context) {
	std::lock_guard<std::mutex> lock(_mutex);
	if (_b_stop) {
		return;
	}
	_connections.push(std::move(context));
	_cv.notify_one();
}

void StatusConPool::Close() {
	_b_stop = true;
	_cv.notify_all();
}

//
GetChatServerRsp StatusGrpcClient::GetChatServer(int uid) {
	ClientContext con;
	GetChatServerReq req;
	GetChatServerRsp rsp;

	req.set_uid(uid);

	// 1. 提取连接
	auto stub = _pool->getConn();
	if (!stub) {
		rsp.set_error(ErrorCodes::RPC_Failed);
		return rsp;
	}

	// 2. 🎯 修正：一旦拿到连接，必须立刻、雷打不动地挂上 Defer 回收守卫！
	Defer defer([&stub, this]() {
		_pool->returnConn(std::move(stub));
		});

	// 3. 发起真实的远程 RPC 呼叫
	Status status = stub->GetChatServer(&con, req, &rsp);

	if (status.ok()) {
		//std::cout << rsp.AppendToString();
		// 此时 rsp 依然鲜活，直接返回它，C++17 之后的 RVO (返回值优化) 会保证其安全安全交付
		return rsp;
	}
	else {
		std::cerr << "gRPC Status Error: " << status.error_message() << std::endl;
		rsp.set_error(ErrorCodes::RPC_Failed);
		return rsp;
	}
}

StatusGrpcClient::StatusGrpcClient() {
	auto& gcfg = ConfigMgr::Inst();
	std::string host = gcfg["StatusServer"]["Host"];
	std::string port = gcfg["StatusServer"]["Port"];
	_pool.reset(new StatusConPool(host, port, 10));
}