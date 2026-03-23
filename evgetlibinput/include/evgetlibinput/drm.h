/**
 * \file drm.h
 * \brief DRM output screen dimension query.
 */

#ifndef EVGETLIBINPUT_DRM_H
#define EVGETLIBINPUT_DRM_H

#include <xf86drm.h>

#include <memory>
#include <vector>

#include "evget/error.h"

namespace evgetlibinput {

/**
 * \brief Screen dimensions from querying the DRM.
 */
struct ScreenDimensions {
    std::uint32_t width;
    std::uint32_t height;
};

/**
 * \brief An interface for querying DRM for screen dimensions.
 */
class DrmApi {
public:
    DrmApi() = default;
    virtual ~DrmApi() = default;

    DrmApi(DrmApi&&) noexcept = delete;
    DrmApi& operator=(DrmApi&&) noexcept = delete;

    DrmApi(const DrmApi&) = delete;
    DrmApi& operator=(const DrmApi&) = delete;

    /**
     * \brief Get the screen dimensions from the DRM. Finds the largest dimension when multiple screens are detected.
     * \return screen dimensions
     */
    virtual evget::Result<ScreenDimensions> GetDimensions() = 0;
};

/**
 * \brief Queries DRM for screen dimensions.
 *
 * Opens `/dev/dri/card*` devices to determine screen dimensions.
 */
class DrmOutput : public DrmApi {
public:
    /**
     * \brief Create a new instance of the DrmOutput.
     * \return DrmOutput
     */
    static evget::Result<std::unique_ptr<DrmOutput>> New();

    evget::Result<ScreenDimensions> GetDimensions() override;

private:
    /**
     * \brief A wrapper around a file descriptor to manager open and close automatically.
     */
    struct File {
        /**
         * \brief Open a path and manage the file descriptor.
         * \param path path to open
         * \return error if file failed to open or the file result
         */
        static evget::Result<std::unique_ptr<File>> Open(const char* path);

        /**
         * \brief Get the file descriptor.
         * \return file descriptor
         */
        [[nodiscard]] int GetFile() const;

        ~File();

        File(File&&) noexcept = delete;
        File& operator=(File&&) noexcept = delete;

        File(const File&) = delete;
        File& operator=(const File&) = delete;

    private:
        explicit File(int file);

        int file_;
    };

    /**
     * \brief A custom deleter for device pointers.
     */
    struct DrmDevicesDeleter {
        /**
         * \brief Create a drm devices delete.
         * \param count number of device pointers
         */
        explicit DrmDevicesDeleter(int count);

        /**
         * \brief Set the count after construction.
         * \param count count
         */
        void SetCount(int count);

        DrmDevicesDeleter() = default;

        void operator()(drmDevicePtr* devices) const {
            drmFreeDevices(devices, count_);
        }

    private:
        int count_ = 0;
    };

    DrmOutput() = default;

    evget::Result<void> QueryCard(File& file);

    ScreenDimensions dimensions_ = {
        .width = 0,
        .height = 0,
    };
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_DRM_H
