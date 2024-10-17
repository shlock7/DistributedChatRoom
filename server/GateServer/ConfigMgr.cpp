#include "ConfigMgr.h"

ConfigMgr::ConfigMgr()
{
	// 获取程序执行路径
	boost::filesystem::path current_path = boost::filesystem::current_path();
	boost::filesystem::path config_path = current_path / "config.ini"; // 这里 / 被重载过了
	std::cout << "Config path: " << config_path << std::endl;

	// 使用Boost.PropertyTree来读取INI文件，存到pt中
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);

	// 遍历INI文件中的所有section  
	for (const auto& section_pair : pt)
	{
		// first是节名，如 [GateServer], .second就是下面的树结构 如port=8080
		const std::string& section_name = section_pair.first;
		const boost::property_tree::ptree& section_tree = section_pair.second;

		// 对每个 section_tree，定义一个 section_config 的 map，用来存储节中的键值对
		std::map<std::string, std::string> section_config;
		for (const auto& key_value_pair : section_tree)
		{
			const std::string& key = key_value_pair.first;
			// second也是一个tree结构的，所以需要转成string类型
			const std::string& value = key_value_pair.second.get_value<std::string>();
			section_config[key] = value;
		}

		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;
		_config_map[section_name] = sectionInfo;
	}

	// 输出所有的section和key-value对  
	for (const auto& section_entry : _config_map)
	{
		const std::string& section_name = section_entry.first;
		SectionInfo section_config = section_entry.second;
		std::cout << "[" << section_name << "]" << std::endl;
		for (const auto& key_value_pair : section_config._section_datas) {
			std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
		}
	}
}
