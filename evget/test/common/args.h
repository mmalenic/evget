#ifndef EVGET_TEST_COMMON_ARGV_H
#define EVGET_TEST_COMMON_ARGV_H

#include <string>
#include <vector>

namespace test {

/**
 * \brief Argv holder for CLI tests.
 */
class Args {
public:
    /**
     * \brief Construct from args.
     */
    explicit Args(std::vector<std::string> args);

    /**
     * \brief `argc` as expected by main,
     */
    [[nodiscard]] int Argc() const;

    /**
     * \brief `argv` as expected by main.
     */
    [[nodiscard]] char** Argv();

private:
    std::vector<std::string> storage_;
    std::vector<char*> pointers_;
};

} // namespace test

#endif
