#include "ConfigMgr1.h"
#include <boost>

ConfigMgr1:ConfigMgr1() {
	std::filesystem::path system_path = std::filesystem::current_path();
	std::filesystem::path config_path = system_path / "config.ini";
	boost::property_tree::ptree pt;
	boost::property_tree::ini_parser::read_ini(config_path, pt);
	for (auto section : pt) {
		const std::string& name = section.first;
		boost::property_tree::ptree pserver = section.second;
		std::map<std::string, std::string> section_config;
		for (auto info : pserver) {
			const std::string& var = info.first;
			const std::string& value = info.second;
			section_config[var] = value;
		}
		SectionInfo1 section;
		section._map = section_config;
		_map[name] = section;
	}

}