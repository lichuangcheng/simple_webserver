#if !defined(SIMPLEWEB_HTTP_REQUEST_INCLUDED)
#define SIMPLEWEB_HTTP_REQUEST_INCLUDED


#include <ostream>
#include <string>
#include <map>


namespace simpleweb {


using Headers = std::map<std::string, std::string>;
using Params = std::map<std::string, std::string>;


enum http_request_state {
    REQUEST_STATUS,    //等待解析状态行
    REQUEST_HEADERS,   //等待解析headers
    REQUEST_BODY,      //等待解析请求body
    REQUEST_DONE       //解析完成
};


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

    enum http_request_state cur_state {REQUEST_STATUS};

    bool has_header(const char *key) const;
    void set_header(const char *key, const char *val);
    void set_header(const char *key, const std::string &val);
    bool has_param(const char *key) const;

    bool parse(std::string_view str);
    
    enum http_request_state current_state() const { return cur_state; }
    bool close_connection() const;

    friend std::ostream& operator << (std::ostream &os, const simpleweb::HttpRequest &req);
private:
    bool parse_request_header(std::string_view d);

};


} // namespace simpleweb


#endif // SIMPLEWEB_HTTP_REQUEST_INCLUDED
