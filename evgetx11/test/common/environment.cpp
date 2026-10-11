#include "common/environment.h"

#include <stdlib.h> // NOLINT(hicpp-deprecated-headers,modernize-deprecated-headers)

#include <cstdlib>
#include <optional>
#include <string>
#include <utility>

test::UnsetEnvironment::UnsetEnvironment(std::string name) : name_{std::move(name)} {
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    if (const char* value = std::getenv(name_.c_str()); value != nullptr) {
        previous_ = value;
    }
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    unsetenv(name_.c_str());
}

test::UnsetEnvironment::~UnsetEnvironment() {
    if (previous_.has_value()) {
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        setenv(name_.c_str(), previous_->c_str(), 1);
    }
}
