#include <gtest/gtest.h>

#include <X11/X.h>
#include <X11/Xproto.h>

#include <cstddef>
#include <string>
#include <thread>
#include <vector>

#include "common/error_handler.h"
#include "common/log.h"
// clang-format off
#include "evgetx11/error_router.h"
// clang-format on

TEST(ErrorRouterTest, InstallAndRestoreHandler) {
    test::InstalledErrorHandler() = test::HostErrorHandler;
    test::FakeDisplays displays{};
    evgetx11::ErrorRouter router{test::SetErrorHandler, test::FakeErrorText, test::RoutedErrorHandler};

    router.Register(displays.At(0));
    ASSERT_EQ(test::InstalledErrorHandler(), test::RoutedErrorHandler);
    router.Register(displays.At(1));
    router.Register(displays.At(1));

    router.Unregister(displays.At(0));
    router.Unregister(displays.At(1));
    ASSERT_EQ(test::InstalledErrorHandler(), test::RoutedErrorHandler);
    router.Unregister(displays.At(1));
    ASSERT_EQ(test::InstalledErrorHandler(), test::HostErrorHandler);
}

TEST(ErrorRouterTest, LogErrorOnDisplay) {
    const test::CapturedLog log{};
    test::InstalledErrorHandler() = test::HostErrorHandler;
    test::FakeDisplays displays{};
    evgetx11::ErrorRouter router{test::SetErrorHandler, test::FakeErrorText, test::RoutedErrorHandler};
    auto window_error = test::CreateXErrorEvent(BadWindow, X_GetWindowAttributes);
    auto atom_error = test::CreateXErrorEvent(BadAtom, X_GetAtomName);

    router.Register(displays.At(0));
    ASSERT_EQ(router.Dispatch(displays.At(0), &window_error), 0);
    ASSERT_EQ(router.Dispatch(displays.At(0), &window_error), 0);
    ASSERT_EQ(router.Dispatch(displays.At(0), &atom_error), 0);
    ASSERT_EQ(router.Dispatch(displays.At(1), &window_error), test::kHostHandled);

    router.Unregister(displays.At(0));
    ASSERT_EQ(router.Dispatch(displays.At(0), &window_error), test::kHostHandled);

    ASSERT_EQ(
        log.Lines(),
        (std::vector<std::string>{
            "warning X request 3.0 failed: fake error",
            "debug X request 3.0 failed: fake error",
            "warning X request 17.0 failed: fake error",
        })
    );
}

TEST(ErrorRouterTest, UnknownErrorsWithoutHandler) {
    test::InstalledErrorHandler() = nullptr;
    test::FakeDisplays displays{};
    evgetx11::ErrorRouter router{test::SetErrorHandler, test::FakeErrorText, test::RoutedErrorHandler};
    auto error = test::CreateXErrorEvent(BadWindow, X_GetWindowAttributes);

    router.Register(displays.At(0));
    ASSERT_EQ(router.Dispatch(displays.At(1), &error), 0);
    router.Unregister(displays.At(0));
}

TEST(ErrorRouterTest, ErrrorFromHostAfterEvget) {
    test::InstalledErrorHandler() = test::HostErrorHandler;
    test::FakeDisplays displays{};
    evgetx11::ErrorRouter router{test::SetErrorHandler, test::FakeErrorText, test::RoutedErrorHandler};

    router.Register(displays.At(0));
    test::InstalledErrorHandler() = test::LaterHostErrorHandler;
    router.Unregister(displays.At(0));

    ASSERT_EQ(test::InstalledErrorHandler(), test::LaterHostErrorHandler);
}

TEST(ErrorRouterTest, RegisterHostHandler) {
    test::InstalledErrorHandler() = test::HostErrorHandler;
    test::FakeDisplays displays{};
    evgetx11::ErrorRouter router{test::SetErrorHandler, test::FakeErrorText, test::RoutedErrorHandler};
    test::ChainedRouter() = &router;
    auto error = test::CreateXErrorEvent(BadWindow, X_GetWindowAttributes);

    router.Register(displays.At(0));
    test::InstalledErrorHandler() = test::ChainingErrorHandler;
    router.Unregister(displays.At(0));
    router.Register(displays.At(0));

    ASSERT_EQ(test::InstalledErrorHandler(), test::ChainingErrorHandler);
    ASSERT_EQ(test::InstalledErrorHandler()(displays.At(1), &error), test::kHostHandled);
    ASSERT_EQ(test::InstalledErrorHandler()(displays.At(0), &error), 0);
    router.Unregister(displays.At(0));
    ASSERT_EQ(test::InstalledErrorHandler(), test::ChainingErrorHandler);
}

TEST(ErrorRouterTest, HandlerChains) {
    test::InstalledErrorHandler() = test::ChainingErrorHandler;
    test::FakeDisplays displays{};
    evgetx11::ErrorRouter router{test::SetErrorHandler, test::FakeErrorText, test::RoutedErrorHandler};
    test::ChainedRouter() = &router;
    test::ChainingCalls() = 0;
    auto error = test::CreateXErrorEvent(BadWindow, X_GetWindowAttributes);

    router.Register(displays.At(0));
    ASSERT_EQ(router.Dispatch(displays.At(1), &error), 0);
    ASSERT_EQ(test::ChainingCalls(), 1);
    router.Unregister(displays.At(0));
}

TEST(ErrorRouterTest, RegistrationDestructor) {
    test::InstalledErrorHandler() = test::HostErrorHandler;
    test::FakeDisplays displays{};
    evgetx11::ErrorRouter router{test::SetErrorHandler, test::FakeErrorText, test::RoutedErrorHandler};

    {
        const evgetx11::ErrorRegistration registration{router, displays.At(0)};
        ASSERT_EQ(test::InstalledErrorHandler(), test::RoutedErrorHandler);
    }
    ASSERT_EQ(test::InstalledErrorHandler(), test::HostErrorHandler);
}

TEST(ErrorRouterTest, DispatchFromManyThreads) {
    test::InstalledErrorHandler() = test::HostErrorHandler;
    test::FakeDisplays displays{};
    evgetx11::ErrorRouter router{test::SetErrorHandler, test::FakeErrorText, test::RoutedErrorHandler};
    constexpr int kIterations = 2000;

    std::vector<int> failures(test::FakeDisplays::kCount, 0);
    {
        std::vector<std::jthread> threads{};
        threads.reserve(test::FakeDisplays::kCount);
        for (std::size_t index = 0; index < test::FakeDisplays::kCount; index++) {
            threads.emplace_back([&router, &displays, &failures, index] {
                auto* own = displays.At(index);
                auto* foreign = displays.At((index + 1) % test::FakeDisplays::kCount);
                auto error = test::CreateXErrorEvent(BadWindow, X_GetWindowAttributes);
                for (int i = 0; i < kIterations; i++) {
                    const evgetx11::ErrorRegistration registration{router, own};
                    const auto own_result = router.Dispatch(own, &error);
                    const auto foreign_result = router.Dispatch(foreign, &error);
                    if (own_result != 0 || (foreign_result != 0 && foreign_result != test::kHostHandled)) {
                        failures.at(index)++;
                    }
                }
            });
        }
    }

    ASSERT_EQ(failures, std::vector<int>(test::FakeDisplays::kCount, 0));
    ASSERT_EQ(test::InstalledErrorHandler(), test::HostErrorHandler);
}
