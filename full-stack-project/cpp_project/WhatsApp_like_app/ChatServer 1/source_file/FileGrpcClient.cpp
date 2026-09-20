#include "FileGrpcClient.h"
#include "ConfigMgr.h"

FileGrpcClient::FileGrpcClient() {
	auto& cfg = ConfigMgr::Inst();
	auto host = cfg["FileServer"]["Host"];
	auto port = cfg["FileServer"]["RPCPort"];
	if (host.empty()) {
		std::cerr << "⚠️ [Config Warning] FileServer Host is empty! Fallback to 127.0.0.1" << std::endl;
		host = "127.0.0.1";
	}

	if (port.empty()) {
		std::cerr << "⚠️ [Config Warning] FileServer RPCPort is empty! Fallback to 50062" << std::endl;
		port = "50062"; // 兜底默认值
	}

	_pool = std::make_unique<FileConnPool>(host, port, 5);
	std::cout << "Connect to the fileserver at " << host << ":" << port << '\n';


}

ApplyUploadRsp FileGrpcClient::ApplyUploadTicket(int from_uid, int to_uid, string file_name, int64_t file_size, string md5) {
	ClientContext con;
	auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
	con.set_deadline(deadline);

	ApplyUploadReq req;
	ApplyUploadRsp rsp;
	req.set_fromuid(from_uid);
	req.set_touid(to_uid);
	req.set_filename(file_name);
	req.set_filesz(file_size);
	req.set_md5(md5);

	auto stub = _pool->getConn();
	if (stub == nullptr) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}
	Defer defer([&stub, this]() {
		_pool->returnConn(std::move(stub));
		});
	Status status=stub->ApplyUploadTicket(&con, req, &rsp);
	if (!status.ok()) {
		std::cerr << "❌ [FileGrpcClient] gRPC call failed! Code: " << status.error_code()
			<< ", Message: " << status.error_message()
			<< ", Details: " << status.error_details() << std::endl;
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}
	std::cout << "✅ [FileGrpcClient] gRPC call succeeded! Token: " << rsp.token() << std::endl;
	return rsp;
}