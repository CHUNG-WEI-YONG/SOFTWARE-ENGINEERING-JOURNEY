#include "const.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

using message::LoginReq;
using message::LoginRsp;

//struct ChatServer {
//	std::string host;
//	std::string port;
//	int conn_count;
//	std::string name;
//};
//class StatusServiceImpl final:public StatusService::Service {
//public:
//	StatusServiceImpl();
//	Status GetChatServer(ServerContext* context, const GetChatServerReq* request,GetChatServerRsp* reply) override;
//	
//
//private:
//	std::unordered_map<std::string,ChatServer> _servers;
//	std::unordered_map<int, std::string> _tokens;
//	int _server_index;
//	std::mutex _servers_mutex;
//	std::mutex _tokens_mutex;
//	std::condition_variable _cv;
//	ChatServer getChatServer();
//	void InsertToken(int uid, std::string token);
//
//};
struct ChatServer {
    std::string name;
    std::string host;
    std::string port;
    int conn_count=0;
};
class StatusServiceImpl final : public StatusService::Service
{
public:
    StatusServiceImpl();
    Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
        GetChatServerRsp* reply) override;
    std::unordered_map <std::string,ChatServer> _servers;
    int _server_index;
    Status Login(ServerContext* context, const LoginReq* request,LoginRsp* reply) override;

private:
        ChatServer getChatServer();
        std::mutex _ChatServerLock;
        void InsertToken(int uid, std::string token);
};