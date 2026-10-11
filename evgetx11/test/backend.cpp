#include <gtest/gtest.h>

#include <optional>

#include "common/environment.h"
#include "common/store.h"
// clang-format off
#include "evgetx11/backend.h"
// clang-format on

TEST(BackendTest, CreateReportsTheDefaultDisplayWhenItCannotOpenIt) {
    const test::UnsetEnvironment display{"DISPLAY"};
    test::StoreMock storage{};

    auto backend = evgetx11::Backend::Create(storage, std::nullopt);

    ASSERT_FALSE(backend.has_value());
    ASSERT_EQ(backend.error().message, "unable to open X11 display ''");
}
