#include "simpleweb/buffer.h"
#include <string.h>
#include <sys/uio.h>

#define INIT_BUFFER_SIZE 65536

namespace simpleweb {

buffer::buffer(size_t size) 
{
    data.resize(size);
    readIndex = 0;
    writeIndex = 0;
}

buffer::~buffer()
{
}

void buffer::make_room(size_t size)
{
    if (writeable() > size)
        return;
        
    //如果front_spare和writeable的大小加起来可以容纳数据，则把可读数据往前面拷贝
    if (spare() + writeable() >= size) {
        auto readable_size = readable();
        for (size_t i = 0; i < readable_size; i++) {
            memcpy(data.data() + i, data.data() + readIndex + i, 1);
        }
        readIndex = 0;
        writeIndex = readable_size;
    }
    else {
        data.resize(data.size() + size);
    }
}

void buffer::append(void *data, int size) 
{
    if (data) {
        make_room(size);
        memcpy(this->data.data() + writeIndex, data, size);
        writeIndex += size;
    }
}

void buffer::append(char c) 
{
    make_room(1);
    data[writeIndex++] = c;
}

void buffer::append(const std::string &s) 
{
    if (!s.empty()) {
        make_room(s.size());
        data.insert(writeIndex, s);
        writeIndex += s.size();
    }
}

int buffer::socket_read(int fd) 
{
    char additional_buffer[INIT_BUFFER_SIZE];
    struct iovec vec[2];
    auto max_writable = writeable();
    vec[0].iov_base = this->data.data() + this->writeIndex;
    vec[0].iov_len = max_writable;
    vec[1].iov_base = additional_buffer;
    vec[1].iov_len = sizeof(additional_buffer);
    auto result = readv(fd, vec, 2);
    if (result < 0) {
        return -1;
    } else if (static_cast<size_t>(result) <= max_writable) {
        writeIndex += result;
    } else {
        writeIndex = data.size();
        append(additional_buffer, result - max_writable);
    }
    return result;
}

} // namespace simpleweb
