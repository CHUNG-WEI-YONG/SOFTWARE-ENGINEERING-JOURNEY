#include "HttpConnection1.h"

HttpConnection1::HttpConnection1(boost::asio::ip::tcp::socket& soc, std::size_t timer):_socket(std::move(soc)),_timer(_socket.get_executor) {
	
}

void HttpConnection1::Start() {
	auto self = shared_from_this()
		async_read(_socket,_buf, _req,[self](const boost::system::error_code& ec, std::size_t bytes_transferred) {
		if (ec) {
			self->_socket.close();
		}
		else {
			self->HandleConn(ec, byte_transferred);
            self->CheckConn();
		}
			});
}

void HttpConnection1::HandleConn(const boost::system::error_code& ec, std::size_t bytes_transferred) {
	std::string_view request_data(_buf.data(), bytes_transferred);
	auto logic = LogicSystem::GetInstance();
	bool success;
	if (request_data.starts_with("GET")) {
		success=logic->HandleGet(_buf);
	}
	else {
		success=logic->HandlePost(_buf);
	}
    if (success) {
        _rsp.set(http:success);
        _rsp.set("gateserver");
        WriteRsp();

    }
	
	
}

void HttpConnection1::WriteRsp(const boost::system::error_code& ec, std::size_t bytes_transferred) {
	auto self = std::shared_from_this();
	async_write(_socket, _buf, [self](const boost::system::error_code& ec, std::size_t bytes_transferred) {
		
		});
		
}

std::string UrlEncode(const std::string& str) {
    std::string strTemp = "";
    size_t length = str.length();

    // 性能优化：提前为新字符串开辟物理内存，防止循环中频繁申请内存导致 ASUS TUF 降频卡顿
    strTemp.reserve(length * 1.2);

    for (size_t i = 0; i < length; i++) {
        // 第一关：判断是否为数字、英文字母或常规安全字符
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~')) {
            strTemp += str[i]; // 绝对安全，绿灯直放
        }
        // 第二关：如果是空格，无脑转换成加号，防止网络断行错位
        else if (str[i] == ' ') {
            strTemp += "+";
        }
        // 第三关：重型熔炼区（敏感符号如 &、#、= 或中文、表情包）
        else {
            strTemp += '%'; // 钉下一根百分号钢钉
            // 核心物理位移：拆出高 4 位和低 4 位，分别转成明文字符
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] & 0x0F);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str) {
    std::string strTemp = "";
    size_t length = str.length();
    strTemp.reserve(length); // 扩容

    for (size_t i = 0; i < length; i++) {
        // 机制 1：撞见百分号，抓取后面 2 个 Hex 字符开始逆向熔炼拼装
        if (str[i] == '%') {
            if (i + 2 < length) {
                unsigned char high = FromHex((unsigned char)str[i + 1]);
                unsigned char low = FromHex((unsigned char)str[i + 2]);
                strTemp += (char)((high << 4) | low); // 位移拼回一个 8 位原始字节
                i += 2; // 雷达向前跳跃 2 格
            }
        }
        // 机制 2：撞见加号，一枪还原回原本的“空格”
        else if (str[i] == '+') {
            strTemp += ' ';
        }
        // 机制 3：普通字符，安全通过
        else {
            strTemp += str[i];
        }
    }
    return strTemp;
}

void HttpConnection::PreParseGetParam() {
    // 提取 URI  
    auto uri = _request.target();
    // 查找查询字符串的开始位置（即 '?' 的位置）  
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos) {
        _get_url = uri;
        return;
    }

    _get_url = uri.substr(0, query_pos);
    std::string query_string = uri.substr(query_pos + 1);
    std::string key;
    std::string value;
    size_t pos = 0;
    while ((pos = query_string.find('&')) != std::string::npos) {
        auto pair = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
            value = UrlDecode(pair.substr(eq_pos + 1));
            _get_params[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    // 处理最后一个参数对（如果没有 & 分隔符）  
    if (!query_string.empty()) {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(query_string.substr(0, eq_pos));
            value = UrlDecode(query_string.substr(eq_pos + 1));
            _get_params[key] = value;
        }
    }
}

void HttpConnection::CheckDeadline() {
    auto self = shared_from_this();
    deadline_.async_wait([self](beast::error_code ec) {
        if (!ec) {
            self->_socket.close(ec);
        }
        });

}