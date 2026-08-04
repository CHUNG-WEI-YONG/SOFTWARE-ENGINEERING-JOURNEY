#include "Singleton.h"

class HttpConnection;
class HttpServer :public std::enable_shared_from_this<HttpServer> {
public:
	void HandleConnection(std::shared_ptr<HttpConnection>);
	HttpServer(boost::beast::io_context& ioc, unsigned short port );
	void start();

private:
	boost::asio::io_context& _ioc;
	boost::asio::ip::tcp::acceptor _acceptor;
	boost::asio::ip::tcp::socket _socket;

};