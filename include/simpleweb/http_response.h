#if !defined(SIMPLEWEB_HTTP_RESPONSE_INCLUDED)
#define SIMPLEWEB_HTTP_RESPONSE_INCLUDED


#include "simpleweb/http_request.h"
#include "simpleweb/buffer.h"
#include <ostream>


namespace simpleweb {


struct HttpResponse 
{
    std::string version;
    int status = -1;
    std::string reason;
    Headers headers;
    std::string body;
    std::string location; // Redirect location

    bool has_header(const char *key) const;
    void set_header(const char *key, const std::string &val);

    void set_content(const char *s, size_t n, const char *content_type);
    void set_content(const std::string &s, const std::string &content_type);

    void write(std::ostream &ostr) const;
    std::string to_string() const;

    void write(Buffer *output);
    static const char *status_message(int status);
};


} // namespace simpleweb


#endif // SIMPLEWEB_HTTP_RESPONSE_INCLUDED
