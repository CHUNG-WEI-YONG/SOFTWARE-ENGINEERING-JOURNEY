#include "endpoint.h"
#include <boost/asio.hpp>
#include <iostream>
#include <string>
using namespace boost;
int client_end_point(){
    std::string raw_ip_address="127.4.8.1";
    unsigned short port_num=3333;
    boost::system::error_code ec;
    asio::ip::address ip_address=asio::ip::make_address(raw_ip_address,ec);
    if(ec.value()!=0){
        std::cout<<"Error occur,code is"<<ec.value()<<".Message is "<<ec.message();
        return ec.value();

    }

    asio::ip::tcp::endpoint ep(ip_address,port_num);
    return 0;

}

int server_end_point(){
    unsigned short port_num=3333;
    asio::ip::address ip_address=asio::ip::address_v4::any();
    asio::ip::tcp::endpoint ep(ip_address,port_num);
    return 0;
}

int create_tcp_socket(){
    asio::io_context ioc;
    asio::ip::tcp protocol=asio::ip::tcp::v4();
    asio::ip::tcp::socket soc(ioc);
    return 0;

}

int create_accept_socket(){
    asio::io_context ioc;
    asio::ip::tcp::acceptor a(ioc,asio::ip::tcp::endpoint(asio::ip::tcp::v4(),3333));
    return 0;
}

int bind_acceptor_socket(){
    unsigned short port=3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(),port);
    asio::io_context ioc;
    asio::ip::tcp::acceptor acceptor(ioc,ep.protocol());
    boost::system::error_code ec;
    acceptor.bind(ep,ec);



}

int connect_to_end(){
    std::string raw_ip="192.168.1.124";
    unsigned short port=3333;
    try{
        asio::ip::tcp::endpoint  ep(asio::ip::make_address(raw_ip),port);
        asio::io_context io;
        asio::ip::tcp::socket sock(io,ep.protocol());
        sock.connect(ep);

    }catch(system::system_error &e){
        std::cout<<"Error occur"<<std::endl;
    }
}

int dns_connect_to_end(){}

int accept_connect_to_end(){
    const int BLOCK_SZ=30;
    unsigned short port=3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(),port);
    asio::io_context io;
    try{
        asio::ip::tcp::acceptor acceptor(io,ep.protocol());
        acceptor.bind(ep);
        acceptor.listen(BLOCK_SZ);
        asio::ip::tcp::socket socket(io);
        acceptor.accept(socket);
    }catch(system::system_error &e){
        std::cout<<"Error";
    }
}

void write_to_socket(asio::ip::tcp::socket& soc){
    std::string a="hello world";
    std::size_t size=0;
    while(size!=a.length()){
        size+=soc.write_some(asio::buffer(a.c_str()+size,a.size()-size));
    }
}

int send_data_by_write_some(){
    std::string raw_ip_address="192.168.3.11";
    unsigned short port=3333;
    try{
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address),port);
        asio::io_context ioc;
        asio::ip::tcp::socket soc(ioc,ep.protocol());
        soc.connect(ep);
        write_to_socket(soc);
    }catch(system::system_error &e){
        std::cout<<"Error occur"<<std::endl;
    }
}

void read_from_socket(asio::ip::tcp::socket soc){
    const unsigned int BUF_SIZE=12;
    char buf[BUF_SIZE];
    std::size_t sz=0;
    while(sz!=BUF_SIZE){
        sz+=soc.read_some(asio::buffer(buf+sz,BUF_SIZE-sz));
    }
    return std::string(buf,sz);
}

int read_data_by_read_some(){
    std::string raw_ip_address="192.168.3.11";
    unsigned short port=3333;
    try{
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address),port);
        asio::io_context ioc;
        asio::ip::tcp::socket soc(ioc,ep.protocol());
        soc.connect(ep);
        write_to_socket(soc);
    }catch(system::system_error &e){
        std::cout<<"Error occur"<<std::endl;
    }
}