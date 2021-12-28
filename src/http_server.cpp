#include "simpleweb/http_server.h"
#include "simpleweb/tcp_connection_factory.h"
#include "simpleweb/socket.h"
#include "simpleweb/error.h"
#include "simpleweb/log.h"
#include <unistd.h>
#include <string.h>
#include <assert.h>


namespace simpleweb {


template <typename F>
int handle_EINTR(F f)
{
    while(1)
    {
        int n = f();
        if (n < 0 && errno == EINTR) continue;
        return n;
    }
}

int process_status_line(char *start, char *end, HttpRequest *httpRequest) 
{
    int size = end - start;
    //method
    char *space = (char *)memmem((const void *)start, (end - start), (const void *)" ", 1);
    assert(space != NULL);
    httpRequest->method.insert(0, start, space - start);

    //url
    start = space + 1;
    space = (char *)memmem(start, end - start, " ", 1);
    assert(space != NULL);
    httpRequest->target.insert(0, start, space - start);
    httpRequest->path = httpRequest->target.substr(0, httpRequest->target.find('?'));

    //version
    start = space + 1;
    httpRequest->version.insert(0, start, end - start);
    assert(space != NULL);
    return size;
}

int parse_http_request(Buffer *input, HttpRequest *httpRequest) 
{
    int ok = 1;
    while (httpRequest->cur_state != REQUEST_DONE) {
        if (httpRequest->cur_state == REQUEST_STATUS) {
            char *crlf = input->find_CRLF();
            if (crlf) {
                int request_line_size = process_status_line(input->data.data() + input->readIndex, crlf, httpRequest);
                if (request_line_size) {
                    input->readIndex += request_line_size;  // request line size
                    input->readIndex += 2;  //CRLF size
                    httpRequest->cur_state = REQUEST_HEADERS;
                }
            }
        } else if (httpRequest->cur_state == REQUEST_HEADERS) {
            char *crlf = input->find_CRLF();
            if (crlf) {
                /**
                 *    <start>-------<colon>:-------<crlf>
                 */
                char *start = input->data.data() + input->readIndex;
                int request_line_size = crlf - start;
                char *colon = (char *)memmem(start, request_line_size, ": ", 2);
                if (colon != NULL) {
                    std::string key;
                    std::string value;

                    key.insert(0, start, colon - start);
                    value.insert(0, colon + 2, (crlf - colon) - 2);

                    httpRequest->headers.emplace(std::move(key), std::move(value));
                    input->readIndex += request_line_size;  //request line size
                    input->readIndex += 2;  //CRLF size
                } else {
                    //读到这里说明:没找到，就说明这个是最后一行
                    input->readIndex += 2;  //CRLF size
                    httpRequest->cur_state = REQUEST_DONE;
                }
            }
        }
    }
    return ok;
}

namespace  {

class HttpConnection : public TCPConnection
{
public:
    public:
    HttpConnection(int fd, EventLoop *eventLoop, request_callback fn) 
        : TCPConnection(fd, eventLoop)
        , request_fn_(fn)
    {
    }

    ~HttpConnection()
    {
    }

protected:
    void on_connection_completed() override
    {
        yolanda_msgx("http connection completed, fd = %d", fd());
    }

    void on_connection_closed() override
    {
        yolanda_msgx("http connection closed: fd = %d", fd());
    }

    void on_message(Buffer *input) override
    {
        if (parse_http_request(input, &request) == 0) {
            std::string error_response = "HTTP/1.1 400 Bad Request\r\n\r\n";
            send_data(error_response.data(), error_response.size());
            shutdown();
        }

        //处理完了所有的request数据，接下来进行编码和发送
        if (request.current_state() == REQUEST_DONE) {

            //httpServer暴露的requestCallback回调
            if (request_fn_)
                request_fn_(request, response);

            Buffer buffer;
            response.write(&buffer);
            send_buffer(&buffer);

            if (request.close_connection()) {
                shutdown();
            }

            request = {};
            response = {};
        }
    }

    void on_write_completed() override
    {
        yolanda_msgx("write completed: %s", name.c_str());
    }

private:
    HttpRequest request;
    HttpResponse response;

    request_callback request_fn_;
};


class HTTPConnectionFactoryImpl : public TCPConnectionFactory
{
public:
    HTTPConnectionFactoryImpl(request_callback fn) : request_callback_fn_(fn)
    {
    }

    ~HTTPConnectionFactoryImpl()
    {
    }

    std::shared_ptr<TCPConnection> create_connection(int conn_fd, EventLoop *loop) override
    {
        return std::make_shared<HttpConnection>(conn_fd, loop, request_callback_fn_);
    }

private:
    request_callback request_callback_fn_;
};


} // namespace 


HttpServer::HttpServer(EventLoop *ev_loop, int port, int thread_num) 
    : tcp_svr_(ev_loop, port, thread_num)
{
}


HttpServer& HttpServer::request(request_callback request_process)
{
    request_fn_ = request_process;
    tcp_svr_.set_connection_factory(std::make_shared<HTTPConnectionFactoryImpl>(std::move(request_process)));
    return *this;
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
    tcp_svr_.start();
}


} // namespace simpleweb
