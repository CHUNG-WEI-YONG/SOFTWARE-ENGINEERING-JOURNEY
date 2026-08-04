#include "MsgNode.h"
#include "const.h"
#include <cstring>

RecvNode::RecvNode(int len, short msg_id):MsgNode(len),_msg_id(msg_id){
	
}

SendNode::SendNode(const char* msg, short msg_len, short msg_id):MsgNode(msg_len+HEAD_TOTAL_LEN),_msg_id(msg_id) {
	short msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
	std::memcpy(_data, &msg_id_host, HEAD_ID_LEN);

	short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(msg_len);
	std::memcpy(_data+HEAD_ID_LEN, &max_len_host,HEAD_DATA_LEN);
	if (msg_len > 0 && msg != nullptr) {
		std::memcpy(_data + HEAD_ID_LEN + HEAD_DATA_LEN, msg, msg_len);
	}
}
