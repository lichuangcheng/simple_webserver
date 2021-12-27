#if !defined(SIMPLEWEB_BUFFER_INCLUDED)
#define SIMPLEWEB_BUFFER_INCLUDED

#include <string>

namespace simpleweb {
    
//数据缓冲区
struct Buffer {
    std::string data;
    size_t readIndex;       //缓冲读取位置
    size_t writeIndex;      //缓冲写入位置

    Buffer(size_t size = 64 * 1024);
    ~Buffer();

    size_t writeable() const {
        return data.size() - writeIndex;
    }
    size_t readable() const {
        return writeIndex - readIndex;
    }
    size_t spare() const {
        return readIndex;
    }

    const char* read_begin() const{
        return data.data() + readIndex;
    }

    //往buffer里写数据
    void append(void *data, int size);
    void append(char c);
    void append(const std::string &data);

    //读socket数据，往buffer里写
    int socket_read(int fd);

    //读buffer数据
    char read_char() {
        char c = data[readIndex];
        readIndex++;
        return c;
    }

    //查询buffer数据
    char* find_CRLF() {
        auto crlf = data.find("\r\n", readIndex);
        if (crlf != data.npos) return &data[crlf];
        else return nullptr;
    }

private:
    void make_room(size_t size);
};


} // namespace simpleweb


#endif // SIMPLEWEB_BUFFER_INCLUDED
