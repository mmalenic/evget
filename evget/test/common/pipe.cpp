#include "common/pipe.h"

#include <fcntl.h>
#include <unistd.h>

#include <array>

test::Pipe::Pipe() {
    std::array<int, 2> ends{-1, -1};
    if (pipe2(ends.data(), O_CLOEXEC) == 0) {
        read_.reset(ends.front());
        write_.reset(ends.back());
    }
}

int test::Pipe::ReadEnd() const {
    return read_.get();
}

bool test::Pipe::Write() const {
    constexpr char kByte = 1;
    return write(write_.get(), &kByte, sizeof(kByte)) == sizeof(kByte);
}
