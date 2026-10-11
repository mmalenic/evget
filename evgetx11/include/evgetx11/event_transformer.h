/**
 * \file event_transformer.h
 * \brief X11-specific event transformer for converting input events to evget format.
 */

#ifndef EVGETX11_EVENT_TRANSFORMER_H
#define EVGETX11_EVENT_TRANSFORMER_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <boost/numeric/conversion/cast.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstddef>
#include <cstring>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event_transformer.h"
#include "evget/interval_tracker.h"
#include "evgetx11/event_switch.h"
#include "evgetx11/event_switch_pointer_key.h"
#include "evgetx11/event_switch_touch.h"
#include "evgetx11/input_event.h"
#include "evgetx11/log_once.h"
#include "evgetx11/x11.h"

namespace evgetx11 {
/**
 * \brief Get the device type of injected input from the XI2.
 * \param use the XI2 device use
 * \return device type
 */
evget::DeviceType InjectedDeviceType(int use);

/**
 * \brief Get the device type from XI1.
 * \param type_name the atom name
 * \return the device type
 */
evget::DeviceType Xi1DeviceType(const char* type_name);

/**
 * \brief Warn about each device XInput 2 reports but XInput 1 could not list.
 * \param xi1_devices the devices `ListInputDevices` returned
 * \param xi2_devices the devices `QueryDevice` returned
 * \param log suppresses repeated warnings
 */
void WarnUnlistedDevices(
    std::span<const XDeviceInfo> xi1_devices,
    std::span<const XIDeviceInfo> xi2_devices,
    LogOnce& log
);

/**
 * \brief Transforms X11 InputEvent objects into the evget `Data` format.
 * \tparam Switches Variadic template parameter for event processing types
 */
template <typename... Switches>
class EventTransformer : public evget::EventTransformer<InputEvent> {
public:
    /**
     * \brief Construct an `EventTransformer` with X11 API wrapper and event processors.
     * \param x_wrapper reference to the X11 API wrapper
     * \param x_event_switch main event processor for handling events
     * \param switches additional event processors for specific event types
     */
    EventTransformer(X11Api& x_wrapper, EventSwitch x_event_switch, Switches... switches);

    /**
     * \brief Transform an `InputEvent` into the evget Data format.
     * \param event the input event to transform
     * \return transformed event data
     */
    evget::Data TransformEvent(InputEvent event) override;

private:
    /**
     * \brief Refresh the device information from the X11 system.
     */
    void RefreshDevices();

    /**
     * \brief Remove the devices from a hierarchy change event.
     * \param event the hierarchy event
     */
    void RemoveDevices(const XIHierarchyEvent& event);

    std::reference_wrapper<X11Api> x_wrapper_;
    EventSwitch x_event_switch_;
    std::optional<Time> previous_{std::nullopt};

    std::unordered_map<int, evget::IntervalTracker> device_intervals_;

    std::tuple<Switches...> switches_;
    std::optional<int> pointer_id_;
    LogOnce log_;
};

/**
 * \brief Builder class for constructing EventTransformer instances.
 */
class EventTransformerBuilder {
public:
    /**
     * \brief Configure the builder to handle pointer and key events.
     * \param x_wrapper Reference to the X11 API wrapper
     * \return Reference to this builder for method chaining
     */
    EventTransformerBuilder& PointerKey(X11Api& x_wrapper);

    /**
     * \brief Configure the builder to handle touch events.
     * \return Reference to this builder for method chaining
     */
    EventTransformerBuilder& Touch();

    /**
     * \brief Build the EventTransformer with the configured settings.
     * \param x_wrapper Reference to the X11 API wrapper
     * \return Unique pointer to the constructed EventTransformer
     */
    std::unique_ptr<evget::EventTransformer<InputEvent>> Build(X11Api& x_wrapper) &&;

private:
    std::optional<EventSwitchPointerKey> pointer_key_;
    std::optional<EventSwitchTouch> touch_;
};

template <typename... Switches>
EventTransformer<Switches...>::EventTransformer(X11Api& x_wrapper, EventSwitch x_event_switch, Switches... switches)
    : x_wrapper_{x_wrapper}, x_event_switch_{std::move(x_event_switch)}, switches_{std::move(switches)...} {
    RefreshDevices();
}

template <typename... Switches>
void EventTransformer<Switches...>::RefreshDevices() {
    int n_devices = 0;
    int xi2_n_devices = 0;
    // See caveats about mixing XI1 calls with XI2 code:
    // https://github.com/freedesktop/xorg-xorgproto/blob/master/specs/XI2proto.txt
    // This should capture all devices with ids in the range 0-128.
    auto info = x_wrapper_.get().ListInputDevices(n_devices);
    auto xi2_info = x_wrapper_.get().QueryDevice(xi2_n_devices);

    WarnUnlistedDevices(
        {info.get(), static_cast<std::size_t>(n_devices)},
        {xi2_info.get(), static_cast<std::size_t>(xi2_n_devices)},
        log_
    );

    std::map<int, std::reference_wrapper<const XIDeviceInfo>> xi2_devices{};
    for (int i = 0; i < xi2_n_devices; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        xi2_devices.emplace(xi2_info[i].deviceid, xi2_info[i]);
    }

    for (int i = 0; i < n_devices; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        const auto& device = info[i];
        int device_id = boost::numeric_cast<int>(device.id);

        if (device.use == IsXPointer) {
            this->pointer_id_ = device_id;
        }

        // The hierarchy event refreshes missing devices again.
        const auto xi2 = xi2_devices.find(device_id);
        if (xi2 == xi2_devices.end()) {
            continue;
        }
        const auto& xi2_device = xi2->second.get();
        if (xi2_device.enabled == False) {
            continue;
        }

        // The server marks XTEST devices with XI_PROP_XTEST_DEVICE; they have no XI1 type and replay injected input.
        const bool injected = x_wrapper_.get().IsXTestDevice(device_id);
        if (!injected && device.type == None) {
            continue;
        }

        const evget::DeviceType device_type =
            injected ? InjectedDeviceType(xi2_device.use) : Xi1DeviceType(x_wrapper_.get().AtomName(device.type).get());
        const std::string name = injected ? std::string{kInjectedDeviceName} : std::string{device.name};

        x_event_switch_.RefreshDevices(device_id, pointer_id_, device_type, name, xi2_device);

        // Iterate through switches and refresh devices.
        std::apply(
            [this, &device_id, &device_type, &name, &xi2_device](auto&&... event_switches) {
                ((event_switches
                      .RefreshDevices(device_id, pointer_id_, device_type, name, xi2_device, x_event_switch_)),
                 ...);
            },
            switches_
        );
    }
}

template <typename... Switches>
void EventTransformer<Switches...>::RemoveDevices(const XIHierarchyEvent& event) {
    for (const auto& info : std::span{event.info, static_cast<std::size_t>(event.num_info)}) {
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        if ((info.flags & (XISlaveRemoved | XIMasterRemoved)) != 0) {
            device_intervals_.erase(info.deviceid);
            x_event_switch_.RemoveDevice(info.deviceid);
        }
    }
}

template <typename... Switches>
evget::Data EventTransformer<Switches...>::TransformEvent(InputEvent event) {
    evget::Data data{};
    if (event.HasData()) {
        auto type = event.GetEventType();

        if (type == XI_HierarchyChanged) {
            RemoveDevices(event.ViewData<XIHierarchyEvent>());
        }
        if (type == XI_DeviceChanged || type == XI_HierarchyChanged) {
            RefreshDevices();
            return data;
        }

        auto source_id = event.ViewData<XIRawEvent>().sourceid;
        // Iterate through switches until the first one returns true.
        std::apply(
            [&event, &data, this, source_id](auto&&... event_switches) {
                ((event_switches.SwitchOnEvent(
                     event,
                     data,
                     x_event_switch_,
                     [this, &event, source_id](Time time) {
                         auto interval = device_intervals_[source_id].Interval(time);
                         if (interval.has_value()) {
                             spdlog::trace(
                                 "interval {} with event type {}",
                                 interval.value().count(),
                                 event.GetEventType()
                             );
                         }

                         return interval;
                     }
                 )) ||
                 ...);
            },
            switches_
        );
    }
    return data;
}
} // namespace evgetx11

#endif
