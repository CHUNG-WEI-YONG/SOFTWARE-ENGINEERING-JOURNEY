#pragma once
#include "const.h"
class HttpConnection;

class Cserver:public std::enable_shared_from_this<Cserver>
{
public:
    Cserver(boost::asio::io_context &ioc,unsigned short port);
    void start();
private:
    tcp::acceptor _acceptor;
    boost::asio::io_context& _ioc;
    tcp::socket _socket;
};
//1st class