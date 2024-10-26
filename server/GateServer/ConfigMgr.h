#pragma once
#include "const.h"

struct SectionInfo
{
	SectionInfo() {}
    ~SectionInfo() { _section_datas.clear(); }

    // ��Ҫ�п�������Ϳ�����ֵ
    SectionInfo(const SectionInfo& src)
    {
        _section_datas = src._section_datas;
    }

    // ������ֵ������һ�� =
    SectionInfo& operator = (const SectionInfo& src) {
        if (&src == this)  // �Լ������Լ���
        {
            return *this;
        }
        // �����Լ������Լ�
        this->_section_datas = src._section_datas;
    }

    std::map<std::string, std::string> _section_datas;
    // ���� [] , ��Ϊ������section_data[key] = value����ʽ����
    std::string  operator[](const std::string& key) {
        if (_section_datas.find(key) == _section_datas.end()) {
            return "";
        }
        // ����������һЩ�߽���  
        return _section_datas[key];
    }

};

// ���ConfigMgrʵ���ǹ���Section���������key��value
class ConfigMgr
{
public:
    ~ConfigMgr()
    {
        _config_map.clear();
    }

    SectionInfo operator[](const std::string& section)
    {
        if (_config_map.find(section) == _config_map.end())
        {
            return SectionInfo();
        }
        return _config_map[section];
    }

    // ��������
    ConfigMgr(const ConfigMgr& src)
    {
        _config_map = src._config_map;
    }
    // ������ֵ����Ҫ���� = 
    ConfigMgr& operator = (const ConfigMgr& src)
    {
        if (&src == this)  // �Լ������Լ���
        {
            return *this;
        }

        _config_map = src._config_map;
    }

    static ConfigMgr& Inst()
    {
        // �������ڸ�����ͬ�����ɼ���Χ�Ǿֲ�������
        // C++ 11�󣬶��̷߳��ʾֲ���̬����ֻ�е�һ�η���ʱ���ʼ��
        // ���̰߳�ȫ��
        static ConfigMgr cfg_mgr;
        return cfg_mgr;
    }


private:
    ConfigMgr();
    // ����key��section��value��һ�������key-value��
    std::map<std::string, SectionInfo> _config_map;
};

