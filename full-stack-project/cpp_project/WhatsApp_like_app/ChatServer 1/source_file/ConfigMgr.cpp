#include "ConfigMgr.h"

ConfigMgr::ConfigMgr() {
	std::filesystem::path system_path = std::filesystem::current_path();
	std::filesystem::path config_path = system_path /"config.ini";

	boost::property_tree::ptree pt;
	try {
		boost::property_tree::ini_parser::read_ini(config_path.string(), pt);
	}
	catch (const std::exception& e) {
		std::cerr << "🔴 [配置熔断] 无法加载或解析 config.ini！原因: " << e.what() << std::endl;
		return; // 优雅熔断退出，大坝绝对不崩溃卡死！
	}

	for (const auto& section : pt) {
		const std::string& section_name = section.first;
		boost::property_tree::ptree section_tree = section.second;
		std::map<std::string, std::string> section_config;
		for (auto& key_value : section_tree) {
			const std::string& key = key_value.first;
			const std::string& value = key_value.second.get_value<std::string>();
			section_config[key] = value;
		}
		SectionInfo section_info;
		section_info._section_datas = section_config;
		_config_map[section_name] = section_info;
	}

	//for (const auto& section_entry : _config_map) {
	//	const std::string& section_name = section_entry.first;
	//	SectionInfo section_config = section_entry.second;
	//	std::cout << "[" << section_name << "]" << std::endl;
	//	for (const auto& key_value_pair : section_config._section_datas) {
	//		std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
	//	}
	//}

}