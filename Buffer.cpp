//
// Created by bbkgl on 19-4-2.
//

#include "Buffer.h"
#include <sys/uio.h>

ssize_t Buffer::ReadFd(int fd, int *saved_errno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = WritableBytes();
    vec[0].iov_base = Begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    const ssize_t n = readv(fd, vec, 2);
    if (n < 0)
        *saved_errno = errno;
    else if (n <= writable)
        writer_index_ += n;
    else
    {
        writer_index_ = buffer_.size();
        Append(extrabuf, n - writable);
    }
    return n;
}