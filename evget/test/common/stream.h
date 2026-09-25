#ifndef EVGET_TEST_COMMON_STREAM_H
#define EVGET_TEST_COMMON_STREAM_H

#include <sstream>

namespace test {

/**
 * \brief A string buffer that counts synchronization.
 */
class SyncCountingBuffer : public std::stringbuf {
public:
    /**
     * \brief Get the number of times the buffer was synchronized.
     * \return sync count
     */
    [[nodiscard]] int SyncCount() const;

protected:
    int sync() override;

private:
    int sync_count_{0};
};

} // namespace test

#endif
