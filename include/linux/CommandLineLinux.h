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

#ifndef INPUT_EVENT_RECORDER_COMMANDLINELINUX_H
#define INPUT_EVENT_RECORDER_COMMANDLINELINUX_H

#include <CommandLine.h>

/**
 * Linux specific command line.
 */
class CommandLineLinux : public CommandLine {
public:
    /**
     * Create a CommandLine object.
     *
     * @param argc from main
     * @param argv from main
     */
    CommandLineLinux(int argc, char* argv[]);

    /**
     * Get mouse device path.
     * @return mouse device path
     */
    [[nodiscard]] const fs::path& getMouseDevice() const;

    /**
     * Get key device path.
     * @return key device path
     */
    [[nodiscard]] const fs::path& getKeyDevice() const;

    /**
     * Get touch device path.
     * @return touch device path
     */
    [[nodiscard]] const fs::path& getTouchDevice() const;

    /**
     * Get the list event devices option.
     * @return is list event devices
     */
    [[nodiscard]] bool isListEventDevices() const;

    std::string platformInformation() override;
    void readArgs() override;

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
     * Validates arguments.
     */
    void validateArgs();
};

#endif // INPUT_EVENT_RECORDER_COMMANDLINELINUX_H
