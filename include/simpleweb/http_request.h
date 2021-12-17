#if !defined(SIMPLEWEB_HTTP_REQUEST_INCLUDED)
#define SIMPLEWEB_HTTP_REQUEST_INCLUDED


#include <string>
#include <map>


namespace simpleweb {


using Headers = std::map<std::string, std::string>;
using Params = std::map<std::string, std::string>;


class HttpRequest
{
public:
    std::string method;
    std::string path;
    Headers headers;
    std::string body;

    std::string remote_addr;
    int remote_port = -1;

    // for server
    std::string version;
    std::string target;
    Params params;

    bool has_header(const char *key) const;
    void set_header(const char *key, const char *val);
    void set_header(const char *key, const std::string &val);
    bool has_param(const char *key) const;
};


template <typename Os>
Os& operator << (Os &os, const simpleweb::HttpRequest &req)
{
    os << "method: "  << req.method  << "\n"
       << "url: "     << req.path    << "\n"
       << "version: " << req.version << "\n";

    for (const auto &[name, value] : req.headers)
    {
        os << name << ": " << value << "\n";
    }
    return os;
}


} // namespace simpleweb


#endif // SIMPLEWEB_HTTP_REQUEST_INCLUDED