#pragma once
#include "const.h"

struct SectionInfo
{
	SectionInfo() {}
    ~SectionInfo() { _section_datas.clear(); }

    // 需要有拷贝构造和拷贝赋值
    SectionInfo(const SectionInfo& src)
    {
        _section_datas = src._section_datas;
    }

    // 拷贝赋值就重载一下 =
    SectionInfo& operator = (const SectionInfo& src) {
        if (&src == this)  // 自己拷贝自己了
        {
            return *this;
        }
        // 不是自己拷贝自己
        this->_section_datas = src._section_datas;
    }

    std::map<std::string, std::string> _section_datas;
    // 重载 [] , 因为期望以section_data[key] = value的形式访问
    std::string  operator[](const std::string& key) {
        if (_section_datas.find(key) == _section_datas.end()) {
            return "";
        }
        // 这里可以添加一些边界检查  
        return _section_datas[key];
    }

};

// 这个ConfigMgr实际是管理Section和其包含的key与value
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

    // 拷贝构造
    ConfigMgr(const ConfigMgr& src)
    {
        _config_map = src._config_map;
    }
    // 拷贝赋值，需要重载 = 
    ConfigMgr& operator = (const ConfigMgr& src)
    {
        if (&src == this)  // 自己拷贝自己了
        {
            return *this;
        }

        _config_map = src._config_map;
    }

    static ConfigMgr& Inst()
    {
        // 生命周期跟进程同步，可见范围是局部作用域
        // C++ 11后，多线程访问局部静态变量只有第一次访问时会初始化
        // 是线程安全的
        static ConfigMgr cfg_mgr;
        return cfg_mgr;
    }


private:
    ConfigMgr();
    // 这里key是section，value是一个具体的key-value对
    std::map<std::string, SectionInfo> _config_map;
};

