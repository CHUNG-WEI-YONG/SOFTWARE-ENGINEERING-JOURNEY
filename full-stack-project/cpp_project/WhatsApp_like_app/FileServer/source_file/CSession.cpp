#include "CSession.h"
#include "cserver.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <json/value.h>
#include <json/json.h>

CSession::CSession(boost::asio::io_context& ioc, tcp::socket soc, std::shared_ptr<cserver> server):_ioc(ioc),_socket(std::move(soc)),_server(server)
{
	_session_id = boost::uuids::to_string(boost::uuids::random_generator()());
	std::filesystem::create_directories("./upload_files");
}

CSession::~CSession()
{
	Close();
}

string CSession::GetSessionId()
{
	return _session_id;
}

void CSession::Start()
{
	asyncReadHead();
}

void CSession::Close()
{
	if (_file_stream.is_open()) {
		_file_stream.flush();
		_file_stream.close();
	}

	// 2. 优雅关闭 TCP 套接字
	boost::system::error_code ec;
	if (_socket.is_open()) {
		_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		_socket.close(ec);
	}

	// 3. 从 cserver 的会话池中移除自身（如果 cserver 维护了 session 映射）
	this->_server->ClearSession(_session_id);
}

void CSession::asyncReadHead()
{

	auto self = shared_from_this();
	boost::asio::async_read(_socket, boost::asio::buffer(_head_buf, 4), [this, self](const boost::system::error_code ec, const std::size_t byte_transferred) {
		if (ec) {
			cout << "Error in reading file";
			Close();
			return;
		}

		memcpy(&_msg_id, _head_buf, 2);
		_msg_id = boost::asio::detail::socket_ops::network_to_host_short(_msg_id);

		memcpy(&_token_len, _head_buf + 2, 2);
		_token_len = boost::asio::detail::socket_ops::network_to_host_short(_token_len);


		if (_msg_id == ID_DOWNLOAD_FILE_REQ) {
			handleDownloadRequest();
			return;
		}
		if (_token_len == 0 || _token_len > 1024) {
			std::cerr << "[CSession] 非法的 Token 长度: " << _token_len << std::endl;
			Close();
			return;
		}

		asyncReadBody();
		});
}

void CSession::asyncReadBody()
{
	auto self = shared_from_this();
	auto token_buf = std::make_shared<std::vector<char>>(_token_len);
	boost::asio::async_read(_socket, boost::asio::buffer(token_buf->data(), _token_len), [this, self, token_buf](const boost::system::error_code ec, const std::size_t byte_transfer) {
		if (ec) {
			cout << "Error in reading file";
			Close();
			return;
		}

		_token.assign(token_buf->data(), _token_len);

		std::string key = FILE_UPLOAD + _token;
		std::string task_json;
		bool success = RedisMjr::GetInstance()->Get(key, task_json);
		if (!success) {
			std::cerr << "[CSession] Token 无效或已过期: " << _token << std::endl;
			Close();
			return;
		}

		Json::Value root;
		Json::Reader reader;

		reader.parse(task_json, root);

		_file_total_bytes = root["filesz"].asInt64();
		std::string file_name = root["filename"].asString();
		std::string save_path = "./upload_files/" + file_name;
		_file_stream.open(save_path, std::ios::binary | std::ios::out);
		if (!_file_stream.is_open()) {
			std::cerr << "[CSession] 无法创建文件: " << save_path << std::endl;
			Close();
			return;
		}

		asyncReadChunk();


	});
}

void CSession::asyncReadChunk()
{
	auto self = shared_from_this();
	auto remain_byte = _file_total_bytes - _file_received_bytes;
	if (remain_byte <= 0) {
		cout << "File successfully being read";


		if (_file_stream.is_open()) {
			_file_stream.flush();
			_file_stream.close();
		}
		RedisMjr::GetInstance()->Del(FILE_UPLOAD + _token);
		Close();
		return;
	}

	std::size_t byte_read = static_cast<size_t>(
		std::min<int64_t>(remain_byte, MAX_CHUNK_SIZE)
	);

	boost::asio::async_read(_socket,
		boost::asio::buffer(_chunk_buffer, byte_read), [this, self](const boost::system::error_code ec, const std::size_t byte_transfer) {
			if (ec) {
				cerr << "Error in Reading chunk";
				Close();
				return;
			}
			_file_stream.write(_chunk_buffer, byte_transfer);
			_file_received_bytes += byte_transfer;

			asyncReadChunk();
		}
	);



}

void CSession::handleDownloadRequest()
{
	std::string key = FILE_UPLOAD + _token;
	std::string task_json="";

	bool success = RedisMjr::GetInstance()->Get(key, task_json);
	if (!success) {
		std::cerr << "[CSession] 下载 Token 不存在或已过期: " << _token << std::endl;
		Close();
		return;
	}

	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(task_json, root)) {
		std::cerr << "[CSession] 任务 JSON 解析失败" << std::endl;
		Close();
		return;
	}

	std::string filename = root["filename"].toStyledString();
	_file_total_bytes = root["filesz"].asInt64();
	_file_sent_bytes = 0;
	std::string filepath = "./upload_files/" + filename;
	_read_file_stream.open(filepath, std::ios::binary | std::ios::in);
	if (!_read_file_stream.is_open()) {
		std::cerr << "Cannot open file" << std::endl;
		Close();
		return;
	}

	std::cout << "[CSession] 开始向客户端下发文件: " << filename
		<< "，总大小: " << _file_total_bytes << " Bytes" << std::endl;

	// 3. 启动异步分块发送
	asyncSendChunk();
}

void CSession::asyncSendChunk()
{
	auto self = shared_from_this();
	if (_file_sent_bytes >= _file_total_bytes || _read_file_stream.eof()) {
		if (_read_file_stream.is_open()) {
			_read_file_stream.close();
		}
		Close();
		return;
	}

	int64_t remain_byte = _file_total_bytes - _file_sent_bytes;
	std::streamsize size = static_cast<std::streamsize>(std::min<int64_t>(remain_byte, static_cast<int64_t>(MAX_CHUNK_SIZE)));

	_read_file_stream.read(_send_chunk_buf, size);
	std::streamsize bufsz = _read_file_stream.gcount();
	if (bufsz <= 0) {
		std::cerr << "[CSession] 读取磁盘文件提前遇到 EOF 或错误" << std::endl;
		Close();
		return;
	}

	boost::asio::async_write(_socket, boost::asio::buffer(_send_chunk_buf, bufsz), [this, self](const boost::system::error_code& ec, const std::size_t byte_transfer) {
		if (ec) {
			std::cerr << "[CSession] 下载发送块失败: " << ec.message() << std::endl;
			Close();
			return;
		}

		_file_sent_bytes += byte_transfer;
		asyncSendChunk();
	});

}







