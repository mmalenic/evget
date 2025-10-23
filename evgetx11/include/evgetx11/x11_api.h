/**
 * \file x11_api.h
 * \brief X11 API abstraction layer for window system interaction and input handling.
 */

#ifndef EVGETX11_X11_API_H
#define EVGETX11_X11_API_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "evget/error.h"

// NOLINTBEGIN(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
namespace evgetx11 {

/**
 * \brief Represents the dimensions of an X11 window.
 */
struct XWindowDimensions {
    unsigned int width;   ///< window width in pixels
    unsigned int height;  ///< window height in pixels
};

/// \brief Type alias for X11 event data with custom deleter.
using XEventPointer = std::unique_ptr<XGenericEventCookie, std::function<void(XGenericEventCookie*)>>;

/**
 * \brief Result structure for X11 pointer queries containing position and state information.
 */
struct QueryPointerResult {
    double root_x{};                                                 ///< Root window X coordinate
    double root_y{};                                                 ///< Root window Y coordinate
    std::unique_ptr<unsigned char[], decltype(&XFree)> button_mask;  ///< Button press mask
    XIModifierState modifier_state{};                                ///< Modifier key states
    XIGroupState group_state{};                                      ///< Keyboard group state
    int screen_number{};                                             ///< Screen number
};

/**
 * \brief An interface which wraps X11 library functions.
 * 
 * This interface provides an abstraction over X11 library functions.
 */
class X11Api {
public:
    /**
     * \brief Look up the character representation of a key event.
     *
     * This function calls `Xutf8LookupString` and uses `XLookupString` as a fallback option.
     *
     * \param event the raw X11 key event
     * \param query_pointer pointer query result required for context
     * \param key_sym output parameter for the key symbol
     * \return string representation of the character
     */
    virtual std::string
    LookupCharacter(const XIRawEvent& event, const QueryPointerResult& query_pointer, KeySym& key_sym) = 0;
    
    /**
     * \brief Get the button mapping for a specific device.
     *
     * This function calls `XGetDeviceButtonMapping`.
     *
     * \param device_id the ID of the input device
     * \param map_size size of the button mapping
     * \return unique pointer to the button mapping array
     */
    virtual std::unique_ptr<unsigned char[]> GetDeviceButtonMapping(int device_id, int map_size) = 0;

    /**
     * \brief List all available input devices.
     *
     * This function calls `XListInputDevices`.
     *
     * \param n_devices output parameter for the number of devices
     * \return unique pointer to the device info array
     */
    virtual std::unique_ptr<XDeviceInfo[], decltype(&XFreeDeviceList)> ListInputDevices(int& n_devices) = 0;
    
    /**
     * \brief Query device information for XI2 devices.
     *
     * This function calls `XIQueryDevice`.
     *
     * \param n_devices output parameter for the number of devices
     * \return unique pointer to the XI device info array
     */
    virtual std::unique_ptr<XIDeviceInfo[], decltype(&XIFreeDeviceInfo)> QueryDevice(int& n_devices) = 0;

    /**
     * \brief Get the name of an X11 atom.
     *
     * This function calls `XGetAtomName`.
     *
     * \param atom the atom to get the name for
     * \return unique pointer to the atom name string
     */
    virtual std::unique_ptr<char[], decltype(&XFree)> AtomName(Atom atom) = 0;

    /**
     * \brief Get the currently active window.
     *
     * This function uses the `_NET_ACTIVE_WINDOW` property.
     *
     * \return optional window ID, `nullopt` if no active window
     */
    virtual std::optional<Window> GetActiveWindow() = 0;
    
    /**
     * \brief Get the window that currently has input focus.
     *
     * This function calls `XGetInputFocus`.
     *
     * \return optional window ID, `nullopt` if no focus window
     */
    virtual std::optional<Window> GetFocusWindow() = 0;
    
    /**
     * \brief Get the name or title of a window.
     *
     * This function uses the `_NET_WM_NAME` property, or the `WM_NAME` property as a fallback.
     *
     * \param window the window to get the name for
     * \return optional window name, `nullopt` if unavailable
     */
    virtual std::optional<std::string> GetWindowName(Window window) = 0;
    
    /**
     * \brief Get the size dimensions of a window.
     *
     * This function calls `GetWindowAttributes`.
     *
     * \param window the window to get the size for
     * \return optional window dimensions, `nullopt` if unavailable
     */
    virtual std::optional<XWindowDimensions> GetWindowSize(Window window) = 0;
    
    /**
     * \brief Get the position of a window.
     *
     * This function calls `GetWindowAttributes`.
     *
     * \param window the window to get the position for
     * \return optional window dimensions, `nullopt` if unavailable
     */
    virtual std::optional<XWindowDimensions> GetWindowPosition(Window window) = 0;

    /**
     * \brief Query the current pointer state for a device.
     *
     * This function calls `XIQueryPointer`.
     *
     * \param device_id the ID of the input device
     * \return query result containing pointer state information
     */
    virtual QueryPointerResult QueryPointer(int device_id) = 0;

    /**
     * \brief Get the next event from the X11 event queue.
     *
     * This function calls `XNextEvent`.
     *
     * \return the next X11 event
     */
    virtual XEvent NextEvent() = 0;
    
    /**
     * \brief Get event data from an X11 event.
     *
     * This function calls `XGetEventData`.
     *
     * \param event reference to the X11 event
     * \return unique pointer to the event data
     */
    virtual XEventPointer EventData(XEvent& event) = 0;

    /**
     * \brief Query the XI2 version supported by the server.
     *
     * This function calls `XIQueryVersion`.
     *
     * \param major output parameter for a major version
     * \param minor output parameter for a minor version
     * \return status indicating success or failure
     */
    virtual Status QueryVersion(int& major, int& minor) = 0;
    
    /**
     * \brief select which XI2 events to receive.
     *
     * This function calls `XISelectEvents` and `XSync`.
     *
     * \param mask event mask specifying which events to receive
     */
    virtual void SelectEvents(XIEventMask& mask) = 0;

    X11Api() = default;
    virtual ~X11Api() = default;

    X11Api(X11Api&&) noexcept = delete;
    X11Api& operator=(X11Api&&) noexcept = delete;

    X11Api(const X11Api&) = delete;
    X11Api& operator=(const X11Api&) = delete;

private:
    static constexpr int kMaskBits = 8;
};

/**
 * \brief Concrete implementation of the `X11Api` interface.
 * 
 * This class provides the actual implementation of X11 library function calls,
 * wrapping the low-level X11 API.
 */
class X11ApiImpl : public X11Api {
public:
    /**
     * \brief Construct an `X11ApiImpl` with a display connection.
     * \param display reference to the X11 Display connection
     */
    explicit X11ApiImpl(Display& display);

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

    /**
     * \brief Iterate over set bits in a mask and call the function for each.
     * \param mask pointer to the mask array
     * \param mask_len length of the mask in bytes
     * \param function function to call for each set bit index
     */
    static void OnMasks(const unsigned char* mask, int mask_len, evget::Invocable<void, int> auto&& function);
    
    /**
     * \brief Convert a `KeySym` to its string representation.
     * \param key_sym the KeySym to convert
     * \return string representation of the KeySym
     */
    static std::string KeySymToString(KeySym key_sym);
    
    /**
     * \brief Set specific event bits in a mask.
     * \param mask pointer to the mask array to modify
     * \param events list of event indices to set
     */
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

void X11ApiImpl::OnMasks(const unsigned char* mask, int mask_len, evget::Invocable<void, int> auto&& function) {
    for (int i = 0; i < mask_len * kMaskBits; i++) {
        if (XIMaskIsSet(mask, i)) {
            function(i);
        }
    }
}

} // namespace evgetx11

// NOLINTEND(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)

#endif
