#include <boost/asio.hpp>
#include <vector>

class HttpConnection1 :public std::enable_shared_from_this<HttpConnection1>{
public:
	HttpConnection1(boost::asio::ip::tcp::socket& soc,std::size_t timer=60);
	void CheckConn();
	void HandleConn(const boost::system::error_code& ec ,std::size_t bytes_transferred);
	void WriteRsp(const boost::system::error_code& ec , std::size_t bytes_transferred);
	void Start();
	void PreParseGetParam();
	void CheckDeadline();
private:
	boost::asio::ip::tcp::socket _socket;
	boost::asio::steady_timer _timer;
	std::vector<char> _buf;
	http::request<http::dynamic_body> _req;
	http::request<http::dynamic_body> _rsp;
	net::steady_timer deadline_{
		_socket.get_executor(),std::chrono::seconds(60)
	};

	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;

};