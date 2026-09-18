#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <functional>
#include "const.h"

using boost::asio::ip::tcp;
class cserver;
class CSession:public std::enable_shared_from_this<CSession> {
public:
	CSession(boost::asio::io_context &ioc,tcp::socket soc,std::shared_ptr<cserver> server);
	~CSession();
	string GetSessionId();
    void Start();
    void Close();

private:
	void asyncReadHead();
	void asyncReadBody();
    void asyncReadChunk();

    boost::asio::io_context& _ioc;
    boost::asio::ip::tcp::socket _socket;
    std::shared_ptr<cserver> _server;
    std::string _session_id;
    char _head_buf[4] = {};

    uint16_t _msg_id{ 0 };

    // 协议与文件状态
    uint16_t _token_len{ 0 };
    std::string _token;
    int64_t _file_total_bytes{ 0 };
    int64_t _file_received_bytes{ 0 };
    std::ofstream _file_stream;

    // 64KB 固定读写缓冲区（避免频繁在堆上分配）
    enum { MAX_CHUNK_SIZE = 65536 };
    char _chunk_buffer[MAX_CHUNK_SIZE] = {};

    std::ifstream _read_file_stream;

    // 下载缓冲与状态跟踪
    char _send_chunk_buf[MAX_CHUNK_SIZE];
    int64_t _file_sent_bytes{ 0 };

    // 下载流程的核心异步函数
    void handleDownloadRequest();
    void asyncSendChunk();
};