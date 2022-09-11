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
//

#ifndef EVGET_XEVENTSWITCHPOINTER_H
#define EVGET_XEVENTSWITCHPOINTER_H

#include "XEventSwitch.h"

namespace EvgetX11 {
    class XEventSwitchPointer : public XEventSwitch {
    public:
        explicit XEventSwitchPointer(XWrapper<XEventDeleter>& xWrapper);

        void refreshDevices(int id, EvgetCore::Event::Device device, const std::string &name, const XIDeviceInfo &info) override;

        void addButtonEvent(const XIDeviceEvent& event, std::chrono::nanoseconds timestamp, EvgetCore::Event::Field::DateTime dateTime, std::vector<EvgetCore::Event::Data>& data, EvgetCore::Event::ButtonAction action, int button);
        void addMotionEvent(const XIDeviceEvent& event, std::chrono::nanoseconds timestamp, EvgetCore::Event::Field::DateTime dateTime, std::vector<EvgetCore::Event::Data>& data);

        const std::string &getButtonName(int id, int button) const;

    private:
        void setButtonMap(const XIButtonClassInfo& buttonInfo, int id);

        std::reference_wrapper<XWrapper<XEventDeleter>> xWrapper;
        std::unordered_map<int, std::unordered_map<int, std::string>> buttonMap{};
    };
}

#endif //EVGET_XEVENTSWITCHPOINTER_H
