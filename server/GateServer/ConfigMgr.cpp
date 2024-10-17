#include "ConfigMgr.h"

ConfigMgr::ConfigMgr()
{
	// ��ȡ����ִ��·��
	boost::filesystem::path current_path = boost::filesystem::current_path();
	boost::filesystem::path config_path = current_path / "config.ini"; // ���� / �����ع���
	std::cout << "Config path: " << config_path << std::endl;

	// ʹ��Boost.PropertyTree����ȡINI�ļ����浽pt��
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);

	// ����INI�ļ��е�����section  
	for (const auto& section_pair : pt)
	{
		// first�ǽ������� [GateServer], .second������������ṹ ��port=8080
		const std::string& section_name = section_pair.first;
		const boost::property_tree::ptree& section_tree = section_pair.second;

		// ��ÿ�� section_tree������һ�� section_config �� map�������洢���еļ�ֵ��
		std::map<std::string, std::string> section_config;
		for (const auto& key_value_pair : section_tree)
		{
			const std::string& key = key_value_pair.first;
			// secondҲ��һ��tree�ṹ�ģ�������Ҫת��string����
			const std::string& value = key_value_pair.second.get_value<std::string>();
			section_config[key] = value;
		}

		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;
		_config_map[section_name] = sectionInfo;
	}

	// ������е�section��key-value��  
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
