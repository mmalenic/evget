// Copyright (C) Marko Malenic 2021.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef INPUT_EVENT_RECORDER_EVENTDEVICE_H
#define INPUT_EVENT_RECORDER_EVENTDEVICE_H

#include <filesystem>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

/**
 * Store the paths for event devices.
 */
class EventDevice {
public:
    /**
     * Create event device.
     * @param device device path
     * @param byId by id path
     * @param byPath by path path
     * @param name device name
     * @param capabilities capabilities
     */
    EventDevice(
        fs::path device,
        const std::optional<std::string>& byId,
        const std::optional<std::string>& byPath,
        const std::optional<std::string>& name,
        const std::vector<std::string>& capabilities
    );

    /**
     * Get device.
     * @return device
     */
    [[nodiscard]] const fs::path& getDevice() const;

    /**
     * Get by id.
     * @return by id
     */
    [[nodiscard]] const std::optional<std::string>& getById() const;

    /**
     * Get by path.
     * @return by path
     */
    [[nodiscard]] const std::optional<std::string>& getByPath() const;

    /**
     * Get name.
     * @return name
     */
    [[nodiscard]] const std::optional<std::string>& getName() const;

    /**
     * Get capabilities.
     * @return capabilities
     */
    [[nodiscard]] const std::vector<std::string>& getCapabilities() const;

    /**
     * Get max name size.
     * @return max name size
     */
    static size_t getMaxNameSize();

    /**
     * Set max name size.
     * @param newMaxNameSize max name size
     */
    static void setMaxNameSize(size_t newMaxNameSize);

    /**
     * Get max path size.
     * @return max path size
     */
    static size_t getMaxPathSize();

    /**
     * Set max path size.
     * @param newMaxPathSize max path size
     */
    static void setMaxPathSize(size_t newMaxPathSize);

    bool operator<(const EventDevice& eventDevice) const;
    friend std::ostream& operator<<(std::ostream& os, const EventDevice& deviceLister);

private:
    fs::path device;
    std::optional<std::string> byId;
    std::optional<std::string> byPath;
    std::optional<std::string> name;
    std::vector<std::string> capabilities;
    static size_t maxNameSize;
    static size_t maxPathSize;
};

#endif //INPUT_EVENT_RECORDER_EVENTDEVICE_H
