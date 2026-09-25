#include "common/stream.h"

#include <sstream>

int test::SyncCountingBuffer::SyncCount() const {
    return sync_count_;
}

int test::SyncCountingBuffer::sync() {
    sync_count_++;
    return std::stringbuf::sync();
}
