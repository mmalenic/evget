#include "evgetlibinput/drm.h"

#include <drm_mode.h>
#include <fcntl.h>
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <span>

#include "evget/error.h"

evget::Result<std::unique_ptr<evgetlibinput::DrmOutput>> evgetlibinput::DrmOutput::New() {
    auto drm_output = std::unique_ptr<DrmOutput>(new DrmOutput{});

    const int device_count = drmGetDevices2(0, nullptr, 0);
    if (device_count <= 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to find DRM devices"}
        };
    }

    // Create device pointers automatically managed with custom deleter.
    // NOLINTBEGIN(modernize-avoid-c-arrays, hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    auto devices =
        std::unique_ptr<drmDevicePtr[], DrmDevicesDeleter>(new drmDevicePtr[device_count], DrmDevicesDeleter{});
    // NOLINTEND(modernize-avoid-c-arrays, hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    const int fetched = drmGetDevices2(0, devices.get(), device_count);
    if (fetched <= 0) {
        // No need to update deleter count on error.
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to fetch DRM devices"}
        };
    }
    // Count is only known after calling drmGetDevices2, so set it here to properly free with deleter.
    devices.get_deleter().SetCount(fetched);

    for (const auto& device : std::span{devices.get(), static_cast<std::size_t>(fetched)}) {
        // Only find primary nodes, e.g. /dev/dri/card0
        if ((static_cast<unsigned int>(device->available_nodes) & 1U << DRM_NODE_PRIMARY) == 0) {
            continue;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const char* device_path = device->nodes[DRM_NODE_PRIMARY];
        auto file = File::Open(device_path);
        if (!file.has_value()) {
            return evget::Err{file.error()};
        }

        auto result = drm_output->QueryCard(**file);
        if (!result.has_value()) {
            return evget::Err{result.error()};
        }
    }

    if (drm_output->dimensions_.width == 0 && drm_output->dimensions_.height == 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "no connected DRM outputs found"}
        };
    }

    return drm_output;
}

evget::Result<void> evgetlibinput::DrmOutput::QueryCard(File& file) {
    std::unique_ptr<drmModeRes, decltype(&drmModeFreeResources)> resources{
        drmModeGetResources(file.GetFile()),
        drmModeFreeResources
    };
    if (resources == nullptr) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = "unable to get DRM resources"}
        };
    }

    for (const auto connector :
         std::span{resources->connectors, static_cast<std::uint32_t>(resources->count_connectors)}) {
        std::unique_ptr<drmModeConnector, decltype(&drmModeFreeConnector)> mode_connector{
            drmModeGetConnector(file.GetFile(), connector),
            drmModeFreeConnector
        };
        if (mode_connector == nullptr || mode_connector->connection != DRM_MODE_CONNECTED) {
            continue;
        }

        // Iterate over modes and get preferred mode.
        for (const auto mode :
             std::span{mode_connector->modes, static_cast<std::uint32_t>(mode_connector->count_modes)}) {
            if ((mode.type & DRM_MODE_TYPE_PREFERRED) != 0U) {
                if (mode.hdisplay * mode.vdisplay > dimensions_.width * dimensions_.height) {
                    dimensions_ = ScreenDimensions{
                        .width = mode.hdisplay,
                        .height = mode.vdisplay,
                    };
                }
                break;
            }
        }
    }

    return {};
}

evget::Result<evgetlibinput::ScreenDimensions> evgetlibinput::DrmOutput::GetDimensions() {
    return dimensions_;
}

evgetlibinput::DrmOutput::File::File(int file) : file_{file} {}

evget::Result<std::unique_ptr<evgetlibinput::DrmOutput::File>> evgetlibinput::DrmOutput::File::Open(const char* path) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    auto file = open(path, O_RDONLY | O_CLOEXEC);
    if (file < 0) {
        return evget::Err{
            {.error_type = evget::ErrorType::kEventHandlerError, .message = std::format("failed to open: {}", path)}
        };
    }

    return std::unique_ptr<File>(new File{file});
}

int evgetlibinput::DrmOutput::File::GetFile() const {
    return file_;
}

evgetlibinput::DrmOutput::File::~File() {
    const int result = close(file_);
    if (result != 0) {
        spdlog::warn("failed to close file descriptor");
    }
}

evgetlibinput::DrmOutput::DrmDevicesDeleter::DrmDevicesDeleter(int count) : count_{count} {}

void evgetlibinput::DrmOutput::DrmDevicesDeleter::SetCount(int count) {
    count_ = count;
}
