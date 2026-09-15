/**
 * \file hid_query_api.h
 * \brief HID queries for the event transformer.
 */

#ifndef EVGETWINDOWS_HID_QUERY_API_H
#define EVGETWINDOWS_HID_QUERY_API_H

#include <windows.h>

#include <cstddef>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

#include "evget/event/device_type.h"
#include "evgetwindows/hid_frame.h"

namespace evgetwindows {

/**
 * \brief HID query the transformer goes through.
 */
class HidQueryApi {
public:
    HidQueryApi() = default;
    virtual ~HidQueryApi() = default;

    HidQueryApi(HidQueryApi&&) noexcept = delete;
    HidQueryApi& operator=(HidQueryApi&&) noexcept = delete;

    HidQueryApi(const HidQueryApi&) = delete;
    HidQueryApi& operator=(const HidQueryApi&) = delete;

    /**
     * \brief Classify a Raw Input device.
     * \param device the Raw Input device handle
     * \return the device type, or `kUnknown` if not a HID touch device
     */
    [[nodiscard]] virtual evget::DeviceType ClassifyDevice(HANDLE device) = 0;

    /**
     * \brief Get the axis a device has positions in.
     * \param device the Raw Input device
     * \return the axis, or nullopt if unavailable
     */
    [[nodiscard]] virtual std::optional<HidAxisRange> AxisRange(HANDLE device) = 0;

    /**
     * \brief Decode HID input report.
     * \param device the Raw Input device
     * \param report the report bytes
     * \return the report, or nullopt if it fails to pars
     */
    [[nodiscard]] virtual std::optional<HidReport> DecodeReport(HANDLE device, std::span<const std::byte> report) = 0;

    /**
     * \brief Remove the device state.
     * \param device the Raw Input device
     */
    virtual void EvictDevice(HANDLE device) = 0;
};

/**
 * \brief The concrete HID implementation of the query API.
 */
class HidQuery : public HidQueryApi {
public:
    HidQuery() = default;
    ~HidQuery() override = default;

    HidQuery(HidQuery&&) noexcept = delete;
    HidQuery& operator=(HidQuery&&) noexcept = delete;

    HidQuery(const HidQuery&) = delete;
    HidQuery& operator=(const HidQuery&) = delete;

    [[nodiscard]] evget::DeviceType ClassifyDevice(HANDLE device) override;
    [[nodiscard]] std::optional<HidAxisRange> AxisRange(HANDLE device) override;
    [[nodiscard]] std::optional<HidReport> DecodeReport(HANDLE device, std::span<const std::byte> report) override;
    void EvictDevice(HANDLE device) override;

    /**
     * \brief Derive device capabilities from data bytes.
     * \param preparsed the data bytes
     * \return the capabilities, or nullopt if the descriptor cannot be read
     */
    [[nodiscard]] static std::optional<HidDeviceCaps> CapsFrom(std::span<const std::byte> preparsed);

    /**
     * \brief Decode the input report from data bytes.
     * \param preparsed the data bytes
     * \param caps the derived capabilities
     * \param report the raw report bytes
     * \return the decoded report, or nullopt if the parser failed
     */
    [[nodiscard]] static std::optional<HidReport>
    DecodeWith(std::span<const std::byte> preparsed, const HidDeviceCaps& caps, std::span<const std::byte> report);

private:
    struct DeviceCache {
        std::vector<std::byte> preparsed;
        HidDeviceCaps caps;
        evget::DeviceType device_type{evget::DeviceType::kUnknown};
        bool type_resolved{false};
        bool parse_resolved{false};
        bool parse_valid{false};
        bool warned{false};
    };

    DeviceCache& Entry(HANDLE device);
    DeviceCache* Parsed(HANDLE device);

    std::unordered_map<HANDLE, DeviceCache> devices_;
};

} // namespace evgetwindows

#endif // EVGETWINDOWS_HID_QUERY_API_H
