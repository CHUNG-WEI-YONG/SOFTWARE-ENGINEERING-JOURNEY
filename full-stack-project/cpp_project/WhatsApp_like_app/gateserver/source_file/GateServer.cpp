#include <iostream>
#include "Cserver.h"
#include "ConfigMgr.h"

int main(){
	auto& gcfg=ConfigMgr::Inst();
	std::string port_number_str = gcfg["server"]["port"];
	unsigned short port = atoi(port_number_str.c_str());

    try{
        unsigned short port=static_cast<unsigned short>(8080);
        net::io_context ioc{1};
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec,int signal_number){
            if(ec){
                return;
            }
            ioc.stop();
        });
        auto server=std::make_shared<Cserver>(ioc,port);
        server->start();
        ioc.run();
    }catch(std::exception const &e){
        std::cerr<<"Error "<<e.what()<<std::endl;
        return EXIT_FAILURE;
    }
}
