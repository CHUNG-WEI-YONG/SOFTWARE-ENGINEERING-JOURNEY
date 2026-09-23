#include "const.h"
#include <boost/asio.hpp>
#include "ConfigMgr.h"
#include "RedisMjr.h"
#include <iostream>
#include <thread>
#include <memory>
#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>
#include "FileServiceImpl.h"
#include <json/json.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>


FileServiceImpl::FileServiceImpl() {

}

Status FileServiceImpl::ApplyUploadTicket(ServerContext* context, const ApplyUploadReq* req, ApplyUploadRsp* rsp) {
    if (req->filename().empty() || req->filesz() <= 0) {
        rsp->set_error(ErrorCodes::RPCFailed); // Param_Invalid
        return Status::OK;
    }

    std::string token = boost::uuids::to_string(boost::uuids::random_generator()());


    Json::Value val;
    val["from_uid"] = req->fromuid();
    val["to_uid"] = req->touid();
    val["filename"] = req->filename();
    val["filesz"] = req->filesz();
    val["md5"] = req->md5(); // 记录客户端发来的 MD5

    Json::FastWriter writer;
    std::string json_task = writer.write(val);

    auto key = FILE_UPLOAD + token;
    auto success = RedisMjr::GetInstance()->Set(key, json_task);
    if (!success) {
        rsp->set_error(ErrorCodes::Error_Json);
        return Status::OK;
    }

    auto& cfg = ConfigMgr::Inst();
    rsp->set_error(ErrorCodes::Success);
    rsp->set_ip(cfg["FileServer"]["Host"]);
    rsp->set_token(token);
    rsp->set_port(std::stoi(cfg["FileServer"]["Port"]));

    return Status::OK;


}