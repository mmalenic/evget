/**
 * \file drm.h
 * \brief DRM output screen dimension query.
 */

#ifndef EVGETLIBINPUT_DRM_H
#define EVGETLIBINPUT_DRM_H

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
     * \brief Get the screen dimensions from the DRM.
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
    DrmOutput() = default;

    std::vector<ScreenDimensions> all_dimensions_;
};

} // namespace evgetlibinput

#endif // EVGETLIBINPUT_DRM_H
