/**
 * \file windows_query_api.h
 * \brief Win32 queries for the event transformer.
 */

#ifndef EVGETWINDOWS_WINDOWS_QUERY_API_H
#define EVGETWINDOWS_WINDOWS_QUERY_API_H

#include <windows.h>

#include <array>
#include <optional>
#include <string>

#include "evgetwindows/modifier_tracker.h"

namespace evgetwindows {

/**
 * \brief Focussed window context.
 */
struct FocusWindowInfo {
    std::string name;
    double position_x;
    double position_y;
    double width;
    double height;
};

/**
 * \brief Win32 query the transformer routes through.
 */
class WindowsQueryApi {
public:
    WindowsQueryApi() = default;
    virtual ~WindowsQueryApi() = default;

    WindowsQueryApi(WindowsQueryApi&&) noexcept = delete;
    WindowsQueryApi& operator=(WindowsQueryApi&&) noexcept = delete;

    WindowsQueryApi(const WindowsQueryApi&) = delete;
    WindowsQueryApi& operator=(const WindowsQueryApi&) = delete;

    /**
     * \brief Translate a virtual-key into its character.
     * \param key the virtual key code
     * \param scan_code the scan code
     * \param key_state the key state array
     * \return the UTF-8 character, or nullopt in not available
     */
    [[nodiscard]] virtual std::optional<std::string>
    CharacterFor(UINT key, UINT scan_code, const std::array<BYTE, kKeyStateSize>& key_state) = 0;

    /**
     * \brief Resolve the device name for a Raw Input device.
     * \param device the Raw Input device handle
     * \return the UTF-8 device name, or nullopt if unavailable
     */
    [[nodiscard]] virtual std::optional<std::string> DeviceName(HANDLE device) = 0;

    /**
     * \brief Capture the focussed window's parameters.
     * \return the focussed window context, or nullopt if nothing in focus
     */
    [[nodiscard]] virtual std::optional<FocusWindowInfo> FocusWindow() = 0;

    /**
     * \brief The display the pointer is currently on.
     * \return the display device name, or nullopt if unavailable
     */
    [[nodiscard]] virtual std::optional<std::string> Screen() = 0;

    /**
     * \brief Read the toggle state of a lock key.
     * \param key the virtual key code
     * \return true when the toggle is on
     */
    [[nodiscard]] virtual bool ToggleState(int key) = 0;
};

/**
 * \brief The concrete Win32 implementation of the query API.
 */
class WindowsQuery : public WindowsQueryApi {
public:
    WindowsQuery() = default;
    ~WindowsQuery() override = default;

    WindowsQuery(WindowsQuery&&) noexcept = delete;
    WindowsQuery& operator=(WindowsQuery&&) noexcept = delete;

    WindowsQuery(const WindowsQuery&) = delete;
    WindowsQuery& operator=(const WindowsQuery&) = delete;

    [[nodiscard]] std::optional<std::string>
    CharacterFor(UINT key, UINT scan_code, const std::array<BYTE, kKeyStateSize>& key_state) override;
    [[nodiscard]] std::optional<std::string> DeviceName(HANDLE device) override;
    [[nodiscard]] std::optional<FocusWindowInfo> FocusWindow() override;
    [[nodiscard]] std::optional<std::string> Screen() override;
    [[nodiscard]] bool ToggleState(int key) override;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_WINDOWS_QUERY_API_H
