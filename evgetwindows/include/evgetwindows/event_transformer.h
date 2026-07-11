/**
 * \file event_transformer.h
 * \brief Transformer for the Windows backend that maps raw input events into the evget schema.
 */

#ifndef EVGETWINDOWS_EVENT_TRANSFORMER_H
#define EVGETWINDOWS_EVENT_TRANSFORMER_H

#include <windows.h>

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "evget/device_id.h"
#include "evget/event/concepts.h"
#include "evget/event/data.h"
#include "evget/event/device_type.h"
#include "evget/event/mouse_move.h"
#include "evget/event/schema.h"
#include "evget/event_transformer.h"
#include "evget/input_event.h"
#include "evget/interval_tracker.h"
#include "evgetwindows/modifier_tracker.h"
#include "evgetwindows/raw_event.h"
#include "evgetwindows/windows_query_api.h"

namespace evgetwindows {

/// \brief Event source for the Windows backend.
constexpr std::string_view kEventSourceName{"windows"};

/// \brief Virtual device name for when the Raw Input device is null.
constexpr std::string_view kInjectedDeviceName{"windows-injected"};

/// \brief The evget namespace for Windows device UUIDs.
inline const boost::uuids::uuid kDeviceNamespace =
    boost::uuids::string_generator{}("6f3b2a1c-8d4e-5f6a-9b7c-0d1e2f3a4b5c");

/**
 * \brief Event transformer for the Windows backend.
 */
class EventTransformer : public evget::EventTransformer<evget::InputEvent<RawEvent>> {
public:
    /**
     * \brief Create an event transformer.
     * \param query the Win32 query API
     * \param tracker the modifier state tracker
     */
    EventTransformer(WindowsQueryApi& query, ModifierTracker& tracker);

    evget::Data TransformEvent(evget::InputEvent<RawEvent> event) override;

private:
    struct EventContext {
        evget::TimestampType timestamp;
        evget::DeviceType device_type;
        std::string device_name;
        std::reference_wrapper<const std::string> device_uuid;
        std::string system_event;
    };

    std::reference_wrapper<WindowsQueryApi> query_;
    std::reference_wrapper<ModifierTracker> tracker_;

    evget::DeviceId<std::string> device_ids_;

    std::unordered_map<std::string, LONG> previous_absolute_x_;
    std::unordered_map<std::string, LONG> previous_absolute_y_;
    std::unordered_map<std::string, evget::IntervalTracker> device_intervals_;

    void BuildMouse(evget::Data& data, EventContext& ctx, const RAWMOUSE& mouse);
    void BuildKeyboard(evget::Data& data, EventContext& ctx, const RAWKEYBOARD& keyboard);
    void SetRelativeFromAbsolute(evget::MouseMove& builder, const std::string& device_uuid, LONG abs_x, LONG abs_y);

    static std::uint64_t ToMicros(const evget::TimestampType& timestamp);

    template <evget::BuilderHasBaseFields T>
    T& SetBaseFields(T& builder, const EventContext& ctx, std::uint64_t event_time);

    template <evget::BuilderHasModifier T>
    T& SetModifierValues(T& builder) const;

    template <typename T>
        requires evget::BuilderHasWindowFunctions<T> && evget::BuilderHasScreenFunction<T>
    T& SetWindowFields(T& builder);
};

template <evget::BuilderHasBaseFields T>
T& EventTransformer::SetBaseFields(T& builder, const EventContext& ctx, std::uint64_t event_time) {
    builder.Timestamp(ctx.timestamp)
        .Interval(device_intervals_[ctx.device_uuid.get()].Interval(event_time))
        .Device(ctx.device_type)
        .DeviceName(ctx.device_name)
        .DeviceId(ctx.device_uuid)
        .SystemEvent(ctx.system_event)
        .EventSource(std::string{kEventSourceName});
    SetWindowFields(builder);
    return SetModifierValues(builder);
}

template <evget::BuilderHasModifier T>
T& EventTransformer::SetModifierValues(T& builder) const {
    for (const auto modifier : tracker_.get().ActiveModifiers()) {
        builder.Modifier(modifier);
    }
    return builder;
}

template <typename T>
    requires evget::BuilderHasWindowFunctions<T> && evget::BuilderHasScreenFunction<T>
T& EventTransformer::SetWindowFields(T& builder) {
    auto window = query_.get().FocusWindow();
    if (!window.has_value()) {
        return builder;
    }

    builder.FocusWindowName(window->name)
        .FocusWindowPositionX(window->position_x)
        .FocusWindowPositionY(window->position_y)
        .FocusWindowWidth(window->width)
        .FocusWindowHeight(window->height)
        .Screen(window->screen);
    return builder;
}

} // namespace evgetwindows

#endif // EVGETWINDOWS_EVENT_TRANSFORMER_H
