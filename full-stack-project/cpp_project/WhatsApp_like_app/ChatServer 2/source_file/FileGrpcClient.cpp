#include "FileGrpcClient.h"
#include "ConfigMgr.h"

FileGrpcClient::FileGrpcClient() {
	auto& cfg = ConfigMgr::Inst();
	auto host = cfg["FileServer"]["Host"];
	auto port = cfg["FileServer"]["RPCPort"];
	_pool = std::make_unique<FileConnPool>(host, port, 5);


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
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}
	//rsp.set_error(ErrorCodes::Success);
	return rsp;
}