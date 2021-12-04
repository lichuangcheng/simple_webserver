//
// ini 配置文件的解析实现参考自 https://github.com/pocoproject/poco 项目的 Util/src/IniFileConfiguration.cpp 等相关文件
// 

#include "simpleweb/ini_config.h"
#include "simpleweb/string_utils.h"
#include <fstream>
#include <iostream>
#include <strings.h>


namespace simpleweb {


IniConfig::IniConfig(const std::string& file) 
{
    load(file);
}


IniConfig::IniConfig(std::istream& is) 
{
    load(is);
}


void IniConfig::load(const std::string& file) 
{
    std::ifstream fin(file);
    if (fin.good()) 
        load(fin);
    else 
        throw std::runtime_error("Cannot open file: " + file);
}


void IniConfig::load(std::istream& is) 
{
    section_.clear();
    map_.clear();
    std::string line;
    while(std::getline(is, line))
    {
        if (!line.empty()) parse_line(line);
    }
}


std::string IniConfig::get_string(const std::string& key) 
{
    std::string value;
    if (get_raw(key, value))
        return value;
    else 
        throw std::invalid_argument("Key Not Found: " + key);
}


std::string IniConfig::get_string(const std::string& key, const std::string& default_value) 
{
    std::string value;
    return get_raw(key, value) ? value : default_value;
}


int32_t IniConfig::get_int32(const std::string& key) 
{
    std::string value;
    if (get_raw(key, value))
        return std::stoi(value, 0, 0);
    else 
        throw std::invalid_argument("Key Not Found: " + key);
}


int32_t IniConfig::get_int32(const std::string& key, int32_t default_value) 
{
    std::string value;
    if (get_raw(key, value))
        return std::stoi(value, 0, 0);
    else 
        return default_value;
}


bool IniConfig::get_bool(const std::string& key) 
{
    std::string value;
    if (get_raw(key, value))
        return parse_bool(value);
    else 
        throw std::invalid_argument("Key Not Found: " + key);
}


bool IniConfig::get_bool(const std::string& key, bool default_value) 
{
    std::string value;
    if (get_raw(key, value))
        return parse_bool(value);
    else 
        return default_value;
}


float IniConfig::get_float(const std::string& key) 
{
    std::string value;
    if (get_raw(key, value))
        return std::stof(value);
    else 
        throw std::invalid_argument("Key Not Found: " + key);
}


float IniConfig::get_float(const std::string& key, float default_value) 
{
    std::string value;
    if (get_raw(key, value))
        return std::stof(value);
    else 
        return default_value;
}


bool IniConfig::get_raw(const std::string& key, std::string& value) 
{
    auto it = map_.find(key);
    if (it != map_.end())
    {
        value = it->second;
        return true;
    }
    else return false;
}


void IniConfig::set_raw(const std::string& key, std::string value) 
{
    map_[key] = value;
}


void IniConfig::display() const
{
    using std::cout;
    cout << "IniConfig Display: \n";
    for (auto & [key, value]: map_)
    {
        cout << key << "=" << value << '\n';
    }
    cout.flush();
}


inline bool is_comment(int c)
{
    return c == ';' || c == '#';
}


void IniConfig::parse_line(const std::string& line) 
{
    auto it  = line.begin();
    auto end = line.end();

    while(it != end && std::isspace(*it)) it++;
    if (is_comment(*it))
        return;

    if (*it == '[')
    {
        std::string section;
        it++;
        while(it != end && *it != ']' && !is_comment(*it))
        {
            section += *it;
            ++it;
        }
        if (*it != ']') std::cerr << "*************** parse error: no break ']' ***************" << std::endl;
        section_ = trim(section);
    }
    else 
    {
        std::string key;
        std::string value;
        while(it != end && *it != '=')
        {
            key += *it;
            it++;
        }
        if (*it == '=')
        {
            ++it;
            while(it != end && !is_comment(*it))
            {
                value += *it;
                it++;
            }
        }
        std::string fullkey = section_;
        if (!fullkey.empty()) fullkey += '.';
        fullkey.append(trim(key));
        map_[fullkey] = trim(value);
    }
}


bool IniConfig::parse_bool(const std::string& value)
{
    if (iequals(value, "1"))
        return true;
    else if (iequals(value, "0"))
        return false;
    else if (iequals(value, "true"))
        return true;
    else if (iequals(value, "yes"))
        return true;
    else if (iequals(value, "on"))
        return true;
    else if (iequals(value, "false"))
        return false;
    else if (iequals(value, "no"))
        return false;
    else if (iequals(value, "off"))
        return false;
    else 
        throw std::runtime_error("Cannot convert to boolean: " + value);
        // throw SyntaxException("Cannot convert to boolean", value);
}


} // namespace simpleweb
