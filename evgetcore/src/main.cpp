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

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <evgetcore/EventHandler.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <iostream>
// #include <linux/input.h>
// #include "evgetcore/CommandLine/ParserLinux.h"
// #include "evgetcore/XInputHandler.h"
// #include "evgetcore/EventTransformerLinux.h"
// #include "evgetcore/SystemEventLoopLinux.h"
// #include "../checkinput/include/checkinput/EventDeviceLister.h"
#include "evgetcore/Event/Data.h"
#include "evgetcore/Event/Key.h"
#include "evgetcore/PrintEvents.h"
#include "evgetx11/EventTransformerX11.h"
#include "evgetx11/XDeviceRefresh.h"
#include "evgetx11/XEventSwitchCore.h"
#include "evgetx11/XEventSwitchPointer.h"
#include "evgetx11/XWrapperX11.h"

using namespace std;

int main(int argc, char* argv[]) {
    //    EvgetCore::Event::Key key{};
    //    constexpr auto schema = EvgetCore::Event::Key::generateSchema();
    //
    //    cout << schema.getFields().size() << endl;

    EvgetCore::PrintEvents printEvents{};

    EvgetCore::Event::Schema<2> innerSchema{
        "data_inner_2",
        {EvgetCore::Event::SchemaField::INTERVAL_FIELD, EvgetCore::Event::SchemaField::INTERVAL_FIELD}};
    EvgetCore::Event::Schema<1, EvgetCore::Event::Schema<2>> innerSchema2{
        "data_inner",
        {EvgetCore::Event::SchemaField::INTERVAL_FIELD},
        {innerSchema, false}};
    EvgetCore::Event::Schema<1, EvgetCore::Event::Schema<1, EvgetCore::Event::Schema<2>>> schema{
        "data",
        {EvgetCore::Event::SchemaField::INTERVAL_FIELD},
        {innerSchema2, false}};

    printEvents.defineSchemas(schema);

    EvgetCore::Event::Data data{"data"};
    data.addField("field");
    EvgetCore::Event::Data data1{"data_inner"};
    data1.addField("inner_field");

    EvgetCore::Event::Data data2{"data_inner_2"};
    data2.addField("inner_field_2");
    data2.addField("inner_field_3");

    data1.contains(data2);
    data.contains(data1);

    data.contains(data2);

    printEvents.store(data);

    //    Display* display = XOpenDisplay(nullptr);
    //    EvgetX11::XWrapperX11 wrapper{*display};
    //    EvgetX11::XDeviceRefresh xDeviceRefresh{};
    //    EvgetX11::XEventSwitchPointer xEventSwitchPointer{wrapper, xDeviceRefresh};
    //    EvgetX11::XEventSwitchCore core{wrapper, xEventSwitchPointer, xDeviceRefresh};
    //    EvgetX11::XInputEvent event = EvgetX11::XInputEvent::nextEvent(wrapper);
    //    EvgetX11::EventData data{};
    //
    //    EvgetX11::EventTransformerX11 transformer{wrapper, core};

    //    CliOption::ParserLinux cmd{};
    //    cmd.parseCommandLine(argc, (const char**) argv);
    //
    //    spdlog::set_level(cmd.getLogLevel());
    //
    //    boost::asio::thread_pool pool{};
    //    auto context = pool.get_executor();
    //    evget::PrintEvents<boost::asio::thread_pool::executor_type> storage{context};
    //    Display* display = XOpenDisplay(nullptr);
    //    evget::EventTransformerLinux transformer{*display};
    //    evget::SystemEventLoopLinux eventLoop{context, evget::XInputHandler{*display}};
    //
    //    evget::EventHandler<boost::asio::thread_pool::executor_type, evget::XInputEvent> handler{context, storage,
    //    eventLoop};
    //
    //    boost::asio::co_spawn(context, [&]() { return handler.start(); }, boost::asio::detached);
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