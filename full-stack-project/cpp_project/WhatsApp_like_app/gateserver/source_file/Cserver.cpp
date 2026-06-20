#include "Cserver.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

Cserver::Cserver(boost::asio::io_context &ioc,unsigned short port):_ioc(ioc),
_acceptor(ioc,tcp::endpoint(tcp::v4(),port)),_socket(ioc){

}

void Cserver::start(){
    auto self=shared_from_this();
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
    _acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
        try{
            if(ec){
                self->start();
                return;
            }
            new_con->start();
            self->start();
        }catch(std::exception &exp){
            std::cout<<"Error occur ";
        }
    });
}