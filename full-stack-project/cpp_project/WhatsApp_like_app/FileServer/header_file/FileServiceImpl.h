#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::ServerContext;
using grpc::Status;
using message::ApplyUploadReq;
using message::ApplyUploadRsp;
using message::FileService;

class FileServiceImpl final:public FileService::Service {
public:
	FileServiceImpl();
	~FileServiceImpl() = default;
	
	Status ApplyUploadTicket(ServerContext* context, const ApplyUploadReq* req, ApplyUploadRsp* rsp);


};

