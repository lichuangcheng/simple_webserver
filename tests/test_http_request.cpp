#include <vector>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <optional>

#include "simpleweb/string_utils.h"
#include <simpleweb/tcp_connection_factory.h>
#include <simpleweb/http_server.h>

namespace fs = std::filesystem;
using namespace simpleweb;
using namespace std::placeholders;

std::optional<std::string> file_media_type(const std::string &ext)
{
    static const std::unordered_map<std::string, std::string> media_type = {
        {".css",  "text/css"},
        {".txt",  "text/plain;charset=utf-8"},
        {".html",  "text/html;charset=utf-8"},

        {".gif",  "image/gif"},
        {".png",  "image/png"},
        {".ico",  "image/x-icon"},
        {".jpeg",  "image/jpeg"},

        {".pdf", "application/pdf"},
        {".js", "application/javascript"},
        {".json", "application/json"},

        {".xml", "application/xml"},
        {".gz", "application/gzip"},
        {".zip", "application/zip"},
    };
    auto it = media_type.find(ext);
    if (it != media_type.end()) return it->second;
    return std::nullopt;
}

std::string readfile(const std::string &file)
{
    std::string out;
    std::ifstream fs(file, std::ios_base::binary);
    fs.seekg(0, std::ios_base::end);
    auto size = fs.tellg();
    fs.seekg(0);
    out.resize(static_cast<size_t>(size));
    fs.read(&out[0], static_cast<std::streamsize>(size));
    return out;
}

void not_found(HttpResponse &res)
{
    static const std::string text = "404 NOT FOUND";
    res.status = 404;
    res.set_content(text, "text/html;charset=utf-8");
}

void unimplemented(HttpResponse &res)
{
    res.status = 501;
    res.set_content("Method Not Implemented", "text/html;charset=utf-8");
}

void bad_request(HttpResponse &res)
{
    res.status = 400;
    res.set_content("400 Bad request", "text/html;charset=utf-8");
}

void request_file(const std::string &root, const HttpRequest &req, HttpResponse &res)
{
    auto full_path = root + req.path;
    if (req.path == "/") full_path += "index.html";
    printf("request file: %s \n", full_path.c_str());

    if (!fs::exists(full_path) || !fs::is_regular_file(full_path))
        return not_found(res);

    auto file = readfile(full_path);
    auto type = file_media_type(fs::path(full_path).extension());
    res.set_content(std::move(file), type.value_or("text/plain;charset=utf-8"));
    res.status = 200;
    return;
}

int main(int argc, char const *argv[])
{
    int port = 8080;
    if (argc >= 2) 
        port = std::stoi(argv[1]);

    printf("listen on 0.0.0.0:%d\n", port);
    
    EventLoop ev_loop;
    HttpServer svr(&ev_loop, port, 0);
    svr.request([](HttpRequest &req, HttpResponse &res) {
        if (iequals(req.method, "GET"))
            request_file("../tests", req, res);
        else
            unimplemented(res);

        res.reason = HttpResponse::status_message(res.status);
        res.headers["Server"] = "simpleweb/0.1.0";
    });

    svr.start();
    ev_loop.run();
    return 0;
}
