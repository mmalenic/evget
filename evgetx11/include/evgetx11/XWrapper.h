#ifndef EVGETX11_XWRAPPER_H
#define EVGETX11_XWRAPPER_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
namespace evgetx11 {

struct XWindowDimensions {
    unsigned int width;
    unsigned int height;
};

using XEventPointer = std::unique_ptr<XGenericEventCookie, std::function<void(XGenericEventCookie*)>>;

struct QueryPointerResult {
    double root_x{};
    double root_y{};
    std::unique_ptr<unsigned char[], decltype(&XFree)> button_mask;
    XIModifierState modifier_state{};
    XIGroupState group_state{};
    int screen_number{};
};

/**
 * An interface which wraps X11 library functions.
 */
class XWrapper {
public:
    virtual std::string
    LookupCharacter(const XIRawEvent& event, const QueryPointerResult& query_pointer, KeySym& key_sym) = 0;
    virtual std::unique_ptr<unsigned char[]> GetDeviceButtonMapping(int device_id, int map_size) = 0;

    virtual std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> ListInputDevices(int& n_devices) = 0;
    virtual std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> QueryDevice(int& n_devices) = 0;

    virtual std::unique_ptr<char[], decltype(&XFree)> AtomName(Atom atom) = 0;

    virtual std::optional<Window> GetActiveWindow() = 0;
    virtual std::optional<Window> GetFocusWindow() = 0;
    virtual std::optional<std::string> GetWindowName(Window window) = 0;
    virtual std::optional<XWindowDimensions> GetWindowSize(Window window) = 0;
    virtual std::optional<XWindowDimensions> GetWindowPosition(Window window) = 0;

    virtual QueryPointerResult QueryPointer(int device_id) = 0;

    virtual XEvent NextEvent() = 0;
    virtual XEventPointer EventData(XEvent& event) = 0;

    virtual Status QueryVersion(int& major, int& minor) = 0;
    virtual void SelectEvents(XIEventMask& mask) = 0;

    XWrapper() = default;
    virtual ~XWrapper() = default;

    XWrapper(XWrapper&&) noexcept = delete;
    XWrapper& operator=(XWrapper&&) noexcept = delete;

    XWrapper(const XWrapper&) = delete;
    XWrapper& operator=(const XWrapper&) = delete;

private:
    static constexpr int kMaskBits = 8;
};
}  // namespace evgetx11

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)

#endif
