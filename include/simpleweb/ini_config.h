#ifndef SIMPLEWEB_INICONFIG_INCUDED
#define SIMPLEWEB_INICONFIG_INCUDED


#include <istream>
#include <string>
#include <map>


namespace simpleweb {
    

class IniConfig
{
public:
    IniConfig() = default;
    IniConfig(const std::string& file);
    IniConfig(std::istream& is);

    void load(const std::string& file);
    void load(std::istream& is);

    std::string get_string(const std::string& key);
    std::string get_string(const std::string& key, const std::string& default_value);

    int32_t get_int32(const std::string& key);
    int32_t get_int32(const std::string& key, int32_t default_value);

    bool get_bool(const std::string& key);
    bool get_bool(const std::string& key, bool default_value);

    float get_float(const std::string& key);
    float get_float(const std::string& key, float default_value);

    bool get_raw(const std::string& key, std::string& value);
    void set_raw(const std::string& key, std::string value);
    
    void display() const;

    ~IniConfig() = default;
private:
    void parse_line(const std::string& line);
    bool parse_bool(const std::string& value);

    typedef std::map<std::string, std::string> StringMap;

    std::string section_;
    StringMap map_;
};


} // namespace simpleweb


#endif // SIMPLEWEB_INICONFIG_INCUDED
