// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef EVGET_XEVENTSWITCH_H
#define EVGET_XEVENTSWITCH_H

#include <spdlog/spdlog.h>

#include <utility>

#include "XWrapperX11.h"
#include "evgetcore/Event/MouseClick.h"
#include "evgetcore/Event/MouseMove.h"

namespace EvgetX11 {
/**
 * Check whether the template parameter is a builder with a modifier function.
 */
template <typename T>
concept BuilderHasModifier = requires(T builder, EvgetCore::Event::ModifierValue modifierValue) {
                                 { builder.modifier(modifierValue) } -> std::convertible_to<T>;
                             };

/**
 * Check whether the template parameter is a builder with focus window functions.
 */
template <typename T>
concept BuilderHasWindowFunctions =
    requires(T builder, std::string name, double x, double y, double width, double height) {
        { builder.focusWindowName(name) } -> std::convertible_to<T>;
        { builder.focusWindowPositionX(x) } -> std::convertible_to<T>;
        { builder.focusWindowPositionY(y) } -> std::convertible_to<T>;
        { builder.focusWindowWidth(width) } -> std::convertible_to<T>;
        { builder.focusWindowHeight(height) } -> std::convertible_to<T>;
    };

/**
 * Check whether the template parameter is a builder with a device name function.
 */
template <typename T>
concept BuilderHasDeviceNameFunctions = requires(T builder, std::string deviceName, std::string info) {
                                           { builder.deviceName(deviceName) } -> std::convertible_to<T>;
                                           { builder.info(info) } -> std::convertible_to<T>;
                                       };

class XEventSwitch {
public:
    explicit XEventSwitch(XWrapper& xWrapper, std::unordered_map<std::string, std::string> nameToInfo);

    void refreshDevices(int id, EvgetCore::Event::Device device, const std::string& name, const XIDeviceInfo& info);

    void addButtonEvent(
        const XIDeviceEvent& event,
        EvgetCore::Event::Timestamp dateTime,
        EvgetCore::Event::Data& data,
        EvgetCore::Event::ButtonAction action,
        int button,
        Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );
    void addMotionEvent(
        const XIDeviceEvent& event,
        EvgetCore::Event::Timestamp dateTime,
        EvgetCore::Event::Data& data,
        Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
    );

    const std::string& getButtonName(int id, int button) const;

    /**
     * Get the device with the given id.
     */
    EvgetCore::Event::Device getDevice(int id) const;

    /**
     * Check whether the device with the given id is present.
     */
    bool hasDevice(int id);

    /**
     * Set the modifier state for a builder.
     */
    template <BuilderHasModifier T>
    static T& setModifierValue(int modifierState, T& builder);

    /**
     * Set the window fields for a builder.
     */
    template <BuilderHasWindowFunctions T>
    T& setWindowFields(T& builder);

    /**
     * Set the device name fields for a builder.
     */
    template <BuilderHasDeviceNameFunctions T>
    T& setDeviceNameFields(T& builder, const XIDeviceEvent& event);

    /**
     * Set the button map for a device.
     * @param buttonInfo button info
     * @param id device id
     */
    void setButtonMap(const XIButtonClassInfo& buttonInfo, int id);

private:
    std::reference_wrapper<XWrapper> xWrapper;
    std::unordered_map<int, std::unordered_map<int, std::string>> buttonMap{};
    std::unordered_map<int, EvgetCore::Event::Device> devices{};
    std::unordered_map<int, std::string> idToName{};
    std::unordered_map<std::string, std::string> nameToInfo{};
};

template <BuilderHasModifier T>
T& EvgetX11::XEventSwitch::setModifierValue(int modifierState, T& builder) {
    // Based on https://github.com/glfw/glfw/blob/dd8a678a66f1967372e5a5e3deac41ebf65ee127/src/x11_window.c#L215-L235
    if (modifierState & ShiftMask) {
        builder.modifier(EvgetCore::Event::ModifierValue::Shift);
    }
    if (modifierState & LockMask) {
        builder.modifier(EvgetCore::Event::ModifierValue::CapsLock);
    }
    if (modifierState & ControlMask) {
        builder.modifier(EvgetCore::Event::ModifierValue::Control);
    }
    if (modifierState & Mod1Mask) {
        builder.modifier(EvgetCore::Event::ModifierValue::Alt);
    }
    if (modifierState & Mod2Mask) {
        builder.modifier(EvgetCore::Event::ModifierValue::NumLock);
    }
    if (modifierState & Mod3Mask) {
        builder.modifier(EvgetCore::Event::ModifierValue::Mod3);
    }
    if (modifierState & Mod4Mask) {
        builder.modifier(EvgetCore::Event::ModifierValue::Super);
    }
    if (modifierState & Mod5Mask) {
        builder.modifier(EvgetCore::Event::ModifierValue::Mod5);
    }

    return builder;
}

template <BuilderHasWindowFunctions T>
T& EvgetX11::XEventSwitch::setWindowFields(T& builder) {
    auto window = xWrapper.get().getActiveWindow();

    if (!window.has_value()) {
        spdlog::warn("failed to get active window, falling back on focus window");
        window = xWrapper.get().getFocusWindow();
    }

    if (!window.has_value()) {
        spdlog::warn("failed to get any focus window");
        return builder;
    }

    auto windowName = xWrapper.get().getWindowName(*window);
    auto windowPosition = xWrapper.get().getWindowPosition(*window);
    auto windowSize = xWrapper.get().getWindowSize(*window);

    if (windowName.has_value()) {
        builder.focusWindowName(*windowName);
    }

    if (windowPosition.has_value()) {
        builder.focusWindowPositionX(windowPosition->width);
        builder.focusWindowPositionY(windowPosition->height);
    }

    if (windowSize.has_value()) {
        builder.focusWindowWidth(windowSize->width);
        builder.focusWindowHeight(windowSize->height);
    }

    return builder;
}

template <BuilderHasDeviceNameFunctions T>
T& EvgetX11::XEventSwitch::setDeviceNameFields(T& builder, const XIDeviceEvent& event) {
    auto name = idToName.at(event.deviceid);

    builder.deviceName(name);
    if (nameToInfo.contains(name)) {
        return builder.info(nameToInfo.at(name));
    } else {
        return builder;
    }
}

void EvgetX11::XEventSwitch::addMotionEvent(
    const XIDeviceEvent& event,
    EvgetCore::Event::Timestamp dateTime,
    EvgetCore::Event::Data& data,
    Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    EvgetCore::Event::MouseMove builder{};
    builder.interval(getTime(event.time))
        .timestamp(dateTime)
        .device(getDevice(event.deviceid))
        .positionX(event.root_x)
        .positionY(event.root_y);

    XEventSwitch::setModifierValue(event.mods.effective, builder);
    setWindowFields(builder);

    setDeviceNameFields(builder, event);

    builder.build(data);
}

void EvgetX11::XEventSwitch::addButtonEvent(
    const XIDeviceEvent& event,
    EvgetCore::Event::Timestamp dateTime,
    EvgetCore::Event::Data& data,
    EvgetCore::Event::ButtonAction action,
    int button,
    Util::Invocable<std::optional<std::chrono::microseconds>, Time> auto&& getTime
) {
    EvgetCore::Event::MouseClick builder{};
    builder.interval(getTime(event.time))
        .timestamp(dateTime)
        .device(getDevice(event.deviceid))
        .positionX(event.root_x)
        .positionY(event.root_y)
        .action(action)
        .button(button)
        .name(buttonMap[event.deviceid][button]);
    XEventSwitch::setModifierValue(event.mods.effective, builder);
    setWindowFields(builder);

    setDeviceNameFields(builder, event);

    builder.build(data);
}
}  // namespace EvgetX11

#endif  // EVGET_XEVENTSWITCH_H
