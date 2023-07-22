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

#include <X11/Xlib.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <cstring>
#include <iostream>

#include "clioption/InvalidCommandLineOption.h"
#include "evgetx11/EventTransformerX11.h"
#include "evgetx11/XEventSwitch.h"
#include "evgetx11/XEventSwitchPointerKey.h"
#include "evgetx11/XWrapperX11.h"

int main(int argc, char* argv[]) {

//    #if defined(EVGETX11_HAS_TOUCH_SUPPORT)
//        std::cout << "Touch support is enabled" << std::endl;
//    #endif
//
//    #if !defined(EVGETX11_HAS_TOUCH_SUPPORT)
//        std::cout << "Touch support not is enabled" << std::endl;
//    #endif
//
//    #if defined(EVGETX11_HAS_FEATURE)
//            std::cout << "FEATURE ENABLED" << std::endl;
//    #endif
//
//    #if !defined(EVGETX11_HAS_FEATURE)
//            std::cout << "FEATUER NOT ENABLED" << std::endl;
//    #endif

//    EvgetCore::Event::Graph<int> g{};
//    //    EvgetCore::Event::Key key{};
//    //    constexpr auto schema = EvgetCore::Event::Key::updateSchema();
//    //
//    //    cout << schema.getFields().size() << endl;
//
//    EvgetCore::PrintEvents printEvents{};
//
//    //    data1.contains(data2);
//    //    data.contains(data1);
//    //
//    //    data.contains(data2);
//
//    //    Display* display = XOpenDisplay(nullptr);
//    //    EvgetX11::XWrapperX11 wrapper{*display};
//    //    EvgetX11::XDeviceRefresh xDeviceRefresh{};
//    //    EvgetX11::XEventSwitch xEventSwitchPointer{wrapper, xDeviceRefresh};
//    //    EvgetX11::XEventSwitchPointerKey core{wrapper, xEventSwitchPointer, xDeviceRefresh};
//    //    EvgetX11::XInputEvent event = EvgetX11::XInputEvent::nextEvent(wrapper);
//    //    EvgetX11::EventData data{};
//    //
//    //    EvgetX11::EventTransformerX11 transformer{wrapper, core};
//
//    //    CliOption::ParserLinux cmd{};
//    //    cmd.parseCommandLine(argc, (const char**) argv);
//    //
//    //    spdlog::set_level(cmd.getLogLevel());
//    //
        boost::asio::thread_pool pool{};
        auto context = pool.get_executor();

        Display* display = XOpenDisplay(nullptr);
        EvgetX11::XWrapperX11 xWrapperX11{*display};

        EvgetX11::XEventSwitch xEventSwitch{xWrapperX11, {}};
        EvgetX11::XEventSwitchPointerKey xEventSwitchPointer{xWrapperX11, xEventSwitch};

        EvgetX11::EventTransformerX11 transformer{xWrapperX11, xEventSwitchPointer};

        EvgetX11::XSetMaskCore setCore{};
        EvgetX11::XSetMaskRefresh setRefresh{};

        EvgetX11::XInputHandler xInputHandler{xWrapperX11, {setCore, setRefresh}};

        EvgetX11::EventLoopX11 eventLoop{xInputHandler};

        EvgetCore::EventHandler handler{context, printEvents, transformer, eventLoop};

        while (true) {
            auto event = xInputHandler.getEvent();

            auto transformed = transformer.transformEvent(std::move(event));
        }

        //boost::asio::co_spawn(context, [&]() { return handler.start(); }, boost::asio::detached);
    //
    //    if (cmd.isListEventDevices()) {
    //        evget::EventDeviceLister lister{};
    //        cout << lister;
    //    }
    //
    //    pool.join();
    //    po::options_description cmdlineDesc("Allowed options");
    //    cmdlineDesc.add_options()
    //            ("help", "produce help message")
    //            ("compression", po::_value<int>(), "set compression level")
    //        ;
    //
    //    po::variables_map vm;
    //    po::store(po::parse_command_line(argc, argv, cmdlineDesc), vm);
    //    po::notify(vm);
    //
    //    if (vm.count("help")) {
    //        cout << cmdlineDesc << "\n";
    //        return 1;
    //    }
    //
    //    if (vm.count("compression")) {
    //        cout << "Compression level was set to "
    //             << vm["compression"].as<int>() << ".\n";
    //    } else {
    //        cout << "Compression level was not set.\n";
    //    }

    return 1;
}