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

#ifndef INPUT_EVENT_RECORDER_COMMANDLINELINUX_H
#define INPUT_EVENT_RECORDER_COMMANDLINELINUX_H

#include <CommandLine.h>

static const std::string FILE_OPTION = "file";

class CommandLineLinux : public CommandLine {
public:
    /**
     * Create a CommandLine object.
     *
     * @param argc from main
     * @param argv from main
     */
    CommandLineLinux(int argc, char *argv[]);

    /**
     * Get mouse device path.
     * @return mouse device path
     */
    const fs::path &getMouseDevice() const;

    /**
    * Get key device path.
    * @return key device path
    */
    const fs::path &getKeyDevice() const;

    /**
    * Get touch device path.
    * @return touch device path
    */
    const fs::path &getTouchDevice() const;

    std::string platformInformation() override;

    bool isListEventDevices() const;

private:
    std::tuple<std::string, std::string, std::string> mouseDeviceOption;
    std::tuple<std::string, std::string, std::string> keyDeviceOption;
    std::tuple<std::string, std::string, std::string> touchDeviceOption;
    std::tuple<std::string, std::string, std::string> listEventDevicesOption;

    fs::path mouseDevice;
    fs::path keyDevice;
    fs::path touchDevice;
    bool listEventDevices;

    /**
     * Checks to see with the list-event-devices flag is used alone{}.
     */
    void checkExclusiveOptions();
};


#endif //INPUT_EVENT_RECORDER_COMMANDLINELINUX_H
