#include "HttpConnection.h"
#include "LogicSystem.h"
//2
HttpConnection::HttpConnection(boost::asio::io_context &ioc):_socket(ioc){}
void HttpConnection::start(){
    auto self=shared_from_this();
    http::async_read(_socket,_buffer,_request,[self](beast::error_code ec,std::size_t byte_transferred){
        try{
            if(ec){
                std::cout<<"Error is "<<ec.what()<<std::endl;
                return;
            }
            boost::ignore_unused(byte_transferred);
            self->HandleReq();
            self->CheckDeadline();
        }
        catch(std::exception &exp){
            std::cout<<"error is "<<exp.what()<<std::endl;
        }
    });

}

tcp::socket& HttpConnection::GetSocket() {
    return _socket;
}


unsigned char ToHex(unsigned char x) {
    return x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x) {
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
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
                unsigned char low  = FromHex((unsigned char)str[i + 2]);
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

//handle request
void HttpConnection::HandleReq(){
    _response.version(_request.version());
    _response.keep_alive(false);
    if(_request.method()==http::verb::get){
        PreParseGetParam();
        bool success=LogicSystem::GetInstance()->HandleGet(_get_url,shared_from_this());
        if(!success){
            _response.result(http::status::not_found);
            _response.set(http::field::content_type,"text/plain");
            beast::ostream(_response.body())<<"url not found \t\n";
            WriteResponse();
            return ;
        }
        _response.result(http::status::ok);
        _response.set(http::field::server , "GateServer");
        WriteResponse();
        return ;

    }

    if(_request.method()==http::verb::post){
        bool success=LogicSystem::GetInstance()->HandlePost(_request.target(),shared_from_this());
        if(!success){
            _response.result(http::status::not_found);
            _response.set(http::field::content_type,"text/plain");
            beast::ostream(_response.body())<<"url not found \t\n";
            WriteResponse();
            return ;
        }
        _response.result(http::status::ok);
        _response.set(http::field::server , "GateServer");
        WriteResponse();
        return ;

    }
}


void HttpConnection::WriteResponse(){
    auto self=shared_from_this();
    _response.content_length(_response.body().size());
    http::async_write(_socket,_response,[self](boost::system::error_code ec,std::size_t size_transferred){
        self->_socket.shutdown(boost::asio::socket_base::shutdown_send,ec);
        self->deadline_.cancel();
    });
}

void HttpConnection::CheckDeadline(){
    auto self=shared_from_this();
    deadline_.async_wait([self](beast::error_code ec){
        if(!ec){
            self->_socket.close(ec);
        }
    });

}


