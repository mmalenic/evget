#ifndef EVGET_TEST_COMMON_PIPE_H
#define EVGET_TEST_COMMON_PIPE_H

#include <boost/scope/unique_fd.hpp>

namespace test {

/**
 * \brief A pipe for reading from a file descriptor.
 */
class Pipe {
public:
    /**
     * \brief Create the pipe.
     */
    Pipe();

    /**
     * \brief Get the read end.
     * \return the read end descriptor
     */
    [[nodiscard]] int ReadEnd() const;

    /**
     * \brief Write to the pipe.
     * \return whether the byte was written
     */
    [[nodiscard]] bool Write() const;

private:
    boost::scope::unique_fd read_;
    boost::scope::unique_fd write_;
};

} // namespace test

#endif
