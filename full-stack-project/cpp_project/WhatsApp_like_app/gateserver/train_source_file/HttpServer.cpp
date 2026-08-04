#include "HttpServer.h"


HttpServer::HttpServer(boost::beast::io_context& ioc,unsigned short port) :_ioc(ioc), _acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), ,_socket(ioc) {

}

HttpServer::start() {
	auto self = shared_from_this();
		_acceptor.async_accept(_socket, [self](ErrorCodes ec) {
		if (const boost::system::error_code & ec) {
			self->start();
			return;
		}
		else {
			auto conn = std::make_shared<HttpConnection>(std::move(self->_socket));
			self->HandleConnection(conn);
			conn->start();
			self->start();

		}
			});
}

void HttpServer::HandleConnection(std::shared_ptr<HttpConnection> conn) {

}