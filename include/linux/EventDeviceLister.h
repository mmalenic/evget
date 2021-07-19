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

#ifndef INPUT_EVENT_RECORDER_EVENTDEVICELISTER_H
#define INPUT_EVENT_RECORDER_EVENTDEVICELISTER_H

#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <map>
#include "EventDevice.h"

class EventDeviceLister {
public:
    /**
     * Create an event device lister.
     */
    EventDeviceLister();

    /**
     * list event devices.
     * @return event devices list
     */
    std::vector<EventDevice> listEventDevices();

    /**
     * Get event devices.
     * @return event devices
     */
    [[nodiscard]] const std::vector<EventDevice> &getEventDevices() const;

    friend std::ostream& operator<<(std::ostream& os, const EventDeviceLister& deviceLister);

private:
    const fs::path inputDirectory;
    const fs::path byId;
    const fs::path byPath;
    const fs::path sysClass;
    const fs::path namePath;
    const std::map<int, std::string> eventCodeToName;

    size_t maxNameSize;
    size_t maxPathSize;
    std::vector<EventDevice> eventDevices;

    /**
     * Check the symlink in path to see if they point to entry.
     * @param entry
     * @param path
     * @param msg
     * @return
     */
    static std::optional<fs::path> checkSymlink(const fs::path& entry, const fs::path& path, const std::string& msg) noexcept;

    /**
     * Create the event codes to name map.
     * @return event codes to name map
     */
    [[nodiscard]] static std::map<int, std::string> getEventCodeToName();

    /**
     * Get name.
     * @return name
     */
    [[nodiscard]] std::string getName(const fs::path& device);

    /**
     * Get capabilities. The underlying ioctl calls require elevated privileges.
     * @return name
     */
    [[nodiscard]] std::vector<std::string> getCapabilities(const fs::path& device);
};

#endif //INPUT_EVENT_RECORDER_EVENTDEVICELISTER_H
