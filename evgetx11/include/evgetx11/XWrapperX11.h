#ifndef EVGETX11_XWRAPPERX11_H
#define EVGETX11_XWRAPPERX11_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <evget/error.h>

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <string>

#include "evgetx11/XWrapper.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
namespace evgetx11 {
class XWrapperX11 : public XWrapper {
public:
    explicit XWrapperX11(Display& display);

    std::string
    LookupCharacter(const XIRawEvent& event, const QueryPointerResult& query_pointer, KeySym& key_sym) override;
    std::unique_ptr<unsigned char[]> GetDeviceButtonMapping(int device_id, int map_size) override;

    std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> ListInputDevices(int& n_devices) override;
    std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> QueryDevice(int& n_devices) override;

    std::unique_ptr<char[], decltype(&XFree)> AtomName(Atom atom) override;
    QueryPointerResult QueryPointer(int device_id) override;

    std::optional<Window> GetActiveWindow() override;
    std::optional<Window> GetFocusWindow() override;
    std::optional<std::string> GetWindowName(Window window) override;
    std::optional<XWindowDimensions> GetWindowSize(Window window) override;
    std::optional<XWindowDimensions> GetWindowPosition(Window window) override;

    XEvent NextEvent() override;
    XEventPointer EventData(XEvent& event) override;

    Status QueryVersion(int& major, int& minor) override;
    void SelectEvents(XIEventMask& mask) override;

    static void OnMasks(const unsigned char* mask, int mask_len, evget::Invocable<void, int> auto&& function);
    static std::string KeySymToString(KeySym key_sym);
    static void SetMask(unsigned char* mask, std::initializer_list<int> events);

private:
    struct GetPropertyResult {
        std::uint64_t n_items{};
        Atom type{};
        int size{};
        std::unique_ptr<unsigned char[], decltype(&XFree)> property;
    };

    static constexpr int kMaskBits = 8;

    [[nodiscard]] std::optional<XWindowAttributes> GetWindowAttributes(Window window) const;

    std::optional<Atom> GetAtom(const char* atom_name) const;

    [[nodiscard]] GetPropertyResult GetProperty(Atom atom, Window window) const;

    static std::unique_ptr<_XIC, decltype(&XDestroyIC)> CreateIc(Display& display, XIM xim);

    static constexpr int kUtf8MaxBytes = 4;
    static constexpr int kWindowPropertySize = 32;

    std::reference_wrapper<Display> display_;

    std::unique_ptr<_XIM, decltype(&XCloseIM)> xim_ =
        std::unique_ptr<_XIM, decltype(&XCloseIM)>{XOpenIM(&display_.get(), nullptr, nullptr, nullptr), XCloseIM};
    std::unique_ptr<_XIC, decltype(&XDestroyIC)> xic_ = CreateIc(display_, xim_.get());
};

void evgetx11::XWrapperX11::OnMasks(
    const unsigned char* mask,
    int mask_len,
    evget::Invocable<void, int> auto&& function
) {
    for (int i = 0; i < mask_len * kMaskBits; i++) {
        if (XIMaskIsSet(mask, i)) {
            function(i);
        }
    }
}
}  // namespace evgetx11

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)

#endif
