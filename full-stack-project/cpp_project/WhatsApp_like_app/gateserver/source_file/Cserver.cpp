#include "Cserver.h"
#include "HttpConnection.h"

Cserver::Cserver(boost::asio::io_context &ioc,unsigned short port):_ioc(ioc),
_acceptor(ioc,tcp::endpoint(tcp::v4(),port)),_socket(ioc){

}

void Cserver::start(){
    auto self=shared_from_this();
    _acceptor.async_accept(_socket,[self](beast::error_code ec){
        try{
            if(ec){
                self->start();
                return;
            }
            std::make_shared<HttpConnection>(std::move(self->_socket))->start();
            self->_socket = boost::asio::ip::tcp::socket(self->_ioc.get_executor());
            self->start();
        }catch(std::exception &exp){
            std::cout<<"Error occur ";
        }
    });
}