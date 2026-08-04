#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "ConfigMgr.h"
#include <hiredis/hiredis.h>
#include "RedisMjr.h"
#include "MysqlMgr.h"
#include "AsioIOServicePool.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>       // 👈 必须明确包含它！
#include <grpcpp/server_builder.h>
#include "StatusServiceImpl.h"

void RunServer() {
	ConfigMgr gcfg = ConfigMgr::Inst();
	std::string server_address = gcfg["StatusServer"]["Host"] + ":" + gcfg["StatusServer"]["Port"];
	std::string redishost = gcfg["RedisServer"]["Host"];
	std::string redisport = gcfg["RedisServer"]["Port"];
	std::string password = gcfg["RedisServer"]["Passwd"];
	int redis_int = std::stoi(redisport);
	if (RedisMjr::GetInstance()->Connect(redishost, redis_int, password, 5)) {
		std::cout << "Redis connect correctly at " << redishost << ":" << redisport;
	}
	else {
		std::cout << "Redis cannot connect";
	}
	StatusServiceImpl service;
	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	std::unique_ptr<grpc::Server>server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	boost::asio::io_context ioc;
	boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

	signals.async_wait([&server](const boost::system::error_code err,int signal_number) {
		if (!err) {
			std::cout << "StatusServer close..." << std::endl;
			server->Shutdown();
		}
		});

	std::thread([&ioc]() {ioc.run();}).detach();

	server->Wait();
	ioc.stop();

}

int main(int argc, char** argv) {
	try {
		std::cout << "This is Status Server" << std::endl;
		RunServer();
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}