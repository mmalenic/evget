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

#ifndef INPUT_EVENT_RECORDER_EVENTDEVICELISTER_H
#define INPUT_EVENT_RECORDER_EVENTDEVICELISTER_H

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "InputDevice.h"

namespace ListInputDevices {
class InputDeviceLister {
public:
    /**
     * Create an event device lister.
     */
    InputDeviceLister();

    /**
     * list input devices.
     * @return input devices list
     */
    std::vector<InputDevice> listInputDevices();

    /**
     * Get event devices.
     * @return event devices
     */
    [[nodiscard]] const std::vector<InputDevice>& getInputDevices() const;

    friend std::ostream& operator<<(std::ostream& os, const InputDeviceLister& deviceLister);

private:
    const fs::path inputDirectory;
    const fs::path byId;
    const fs::path byPath;
    const fs::path sysClass;
    const fs::path namePath;
    const std::map<int, std::string> eventCodeToName;

    size_t maxNameSize;
    size_t maxPathSize;
    std::vector<InputDevice> inputDevices;

    /**
     * Check the symlink in path to see if they point to entry.
     * @param entry
     * @param path
     * @param msg
     * @return
     */
    static std::optional<fs::path>
    checkSymlink(const fs::path& entry, const fs::path& path, const std::string& msg) noexcept;

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
    [[nodiscard]] std::vector<std::pair<int, std::string>> getCapabilities(const fs::path& device);
};
}  // namespace ListInputDevices

#endif  // INPUT_EVENT_RECORDER_EVENTDEVICELISTER_H
