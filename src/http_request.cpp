#include "simpleweb/http_request.h"
#include "simpleweb/string_utils.h"

#include <set>

namespace simpleweb {

bool HttpRequest::has_header(const char *key) const
{
    return headers.contains(key);
}

void HttpRequest::set_header(const char *key, const char *val) 
{
    headers[key] = val;
}

void HttpRequest::set_header(const char *key, const std::string &val) 
{
    headers[key] = val;
}

bool HttpRequest::has_param(const char *key) const
{
    return params.contains(key);
}

bool HttpRequest::close_connection() const
{
    static const std::string HTTP10 = "HTTP/1.0";
    static const std::string HTTP11 = "HTTP/1.1";
    static const std::string KEEP_ALIVE = "Keep-Alive";
    static const std::string CLOSE = "close";

    auto it = headers.find("Connection");
    if (it == headers.end()) return true;

    if (CLOSE.compare(it->second) == 0)
        return true;

    if (version == HTTP10 && KEEP_ALIVE.compare(it->second) != 0)
        return true;

    return false;
}

bool HttpRequest::parse_request_header(std::string_view d)
{
    static const std::set<std::string> methods {
      "GET",     "HEAD",    "POST",  "PUT",   "DELETE",
      "CONNECT", "OPTIONS", "TRACE", "PATCH", "PRI"};

    method.clear();
    version.clear();
    target.clear();

    size_t i = 0;
    auto n = d.size();
    while(!std::isspace(d[i]) && i < n) { method += d[i]; i++; }
    if (!methods.contains(simpleweb::to_upper(method))) return false;

    while(std::isspace(d[i])) i++;
    while(!isspace(d[i]) && i < n) { target += d[i]; i++; }

    while(std::isspace(d[i])) i++;
    while(!isspace(d[i]) && i < n) { version += d[i]; i++; }

    // TODO: 解析 params
    path = target.substr(0, target.find('?'));
    return true;
}

bool HttpRequest::parse(std::string_view str)
{
    const auto npos = str.npos;
    auto crlf = str.find("\r\n");
    if (crlf == npos) return false;

    if (!parse_request_header(str.substr(0, crlf))) return false;
    
    auto begin = crlf + 2;
    while(begin < str.size() && (crlf = str.find("\r\n", begin)) != str.npos && crlf != 0)
    {
        std::string_view line = str.substr(begin, crlf - begin);
        auto sep = line.find(':');
        if (sep != line.npos)
        {
            headers.emplace(line.substr(0, sep), trim(line.substr(sep + 1)));
        }
        begin = crlf + 2;
    }
    return true;
}

std::ostream& operator << (std::ostream &os, const simpleweb::HttpRequest &req) 
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
