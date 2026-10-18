#pragma once
#include <string>
#include <vector>

//struct ApplyInfo {
//	ApplyInfo(int uid, std::string name, std::string desc,
//		std::string icon, std::string nick, int sex, int status)
//		:_uid(uid), _name(name), _desc(desc),
//		_icon(icon), _nick(nick), _sex(sex), _status(status) {
//	}
//
//	int _uid;
//	std::string _name;
//	std::string _desc;
//	std::string _icon;
//	std::string _nick;
//	int _sex;
//	int _status;
//};

//聊天线程信息
struct ChatThreadInfo {
    int64_t     _thread_id{ 0 };
    std::string _type;          // "private" 或 "group"

    // 私聊特有字段
    int64_t     _user1_id{ 0 };
    int64_t     _user2_id{ 0 };

    // 群聊特有字段
    std::string _group_name;
    int64_t     _owner_id{ 0 };
    std::string _avatar_url;

    ChatThreadInfo() = default;

    ChatThreadInfo(int64_t thread_id, const std::string& type, int64_t u1 = 0, int64_t u2 = 0)
        : _thread_id(thread_id), _type(type), _user1_id(u1), _user2_id(u2), _owner_id(0) {
    }

    // 辅助方法：快速获取私聊中的对方 UID
    int64_t GetPeerUid(int64_t my_uid) const {
        if (_type != "private") return 0;
        return (_user1_id == my_uid) ? _user2_id : _user1_id;
    }

    bool IsGroup() const {
        return _type == "group";
    }
};

//聊天消息信息
struct ChatMessage {
	int message_id;
	int thread_id;
	int sender_id;
	std::string unique_id;
	std::string content;
	std::string created_at;
    std::string updated_at;
    int status{ 0 };
    int type{ 0 };

    std::string filename;
    int64_t filesz{ 0 };
    std::string token;
    std::string md5;
    std::string file_type;
    std::string file_url;
};

// 查询结果结构，增加next_cursor字段
struct PageResult {
	std::vector<ChatMessage> messages;
	bool load_more;
	int next_cursor;  // 本页最后一条message_id，用于下次查询
};

