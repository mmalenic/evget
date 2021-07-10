/**
 * @author Marko Malenic
 * @date 6/7/21
 */
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

using namespace std;

int main(int argc, char *argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message.")
        ("directory,d", "directory to store events.")
        ("mouse-event-device,d", "directory to store events.")
        ;

    po::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    if (vm.count("directory")) {
        cout << "Compression level was set to "
             << vm["compression"].as<string>() << ".\n";
    } else {
        cout << "Compression level was not set.\n";
    }

}