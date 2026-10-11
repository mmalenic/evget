#ifndef EVGETX11_TEST_COMMON_ENVIRONMENT_H
#define EVGETX11_TEST_COMMON_ENVIRONMENT_H

#include <optional>
#include <string>

namespace test {

/**
 * \brief Unsets an environment variable for the lifetime of this object.
 */
class UnsetEnvironment {
public:
    /**
     * \brief Unset the variable.
     * \param name the variable name
     */
    explicit UnsetEnvironment(std::string name);

    /**
     * \brief Restore the variable if it was set.
     */
    ~UnsetEnvironment();

    UnsetEnvironment(const UnsetEnvironment&) = delete;
    UnsetEnvironment(UnsetEnvironment&&) = delete;
    UnsetEnvironment& operator=(const UnsetEnvironment&) = delete;
    UnsetEnvironment& operator=(UnsetEnvironment&&) = delete;

private:
    std::string name_;
    std::optional<std::string> previous_;
};

} // namespace test

#endif
