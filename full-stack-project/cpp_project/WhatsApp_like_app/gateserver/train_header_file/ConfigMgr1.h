#include "Singleton.h"
#include <unordered_map>

struct SectionInfo1 {
	std::unordered_map<std::string, std::string> _map;
	SectionInfo1()=default;
	std::string operator [](std::string name) {
		auto it = _map.find(name)
			if (it != _map.end()) {
				return it->second;
			}
		return ""
	}
	SectionInfo& operator =(SectionInfo& other) {
		if (&other == this) {
			return *this
		}
		this->_map = other->_map;
		return *this;
	}

};

class ConfigMgr1 : public Singleton<ConfigMgr> {
	friend class Singleton<ConfigMgr>;
	

public:
	SectionInfo1& operator [](std::string name) {
		auto it=_map.find(name)
		if (it!=_map.end()) {
			return it->second;
		}
		return ""
	}

private:
	ConfigMgr1();
	std::unordered_map<std::string, SectionInfo1> _map;

};