//#include <iostream>
//#include <Windows.h>
//#include <csignal>
//#include <chrono>
//#include <boost/program_options.hpp>
//#include <boost/circular_buffer.hpp>
//
//namespace po = boost::program_options;
//using namespace std;
//
//constexpr auto ELEMENTS_TO_AVERAGE = 100;
//
//HHOOK handle;
//auto interval = std::chrono::system_clock::now();
//boost::circular_buffer<double> intervals(ELEMENTS_TO_AVERAGE);
//
//double average() {
//	double sum = 0;
//	for (auto interval : intervals) {
//		sum += interval;
//	}
//	return sum / intervals.size() * 1000;
//}
//
//LRESULT CALLBACK lowLevelMouseProc(
//	_In_ int    nCode,
//	_In_ WPARAM wParam,
//	_In_ LPARAM lParam
//) {
//	const std::chrono::duration<double> difference = std::chrono::system_clock::now() - interval;
//
//	auto *lp = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
//	if (wParam == WM_MOUSEMOVE) {
//		intervals.push_back(difference.count());
//		std::cout << "Point at (" << lp->pt.x << ", " << lp->pt.y;
//		std::cout << ") with interval from previous event: " << difference.count() * 1000 << "ms\n";
//		std::cout << "Estimated polling rate as average: " << average() << "ms\r";
//		std::cout.flush();
//	}
//
//	interval = std::chrono::system_clock::now();
//	return CallNextHookEx(nullptr, nCode, wParam, lParam);
//}
//
//int cleanup() {
//	if (!UnhookWindowsHookEx(handle)) {
//		std::cout << "Error: unable to unhook" << std::endl;
//		return -1;
//	}
//	return 0;
//}
//
//void sigint_handler(const int signum) {
//	std::cout << "sigint received." << std::endl;
//	cleanup();
//	exit(signum);
//}
//
//void low_level_api() {
//	handle = SetWindowsHookExA(WH_MOUSE_LL, &lowLevelMouseProc, NULL, 0);
//	if (handle == nullptr) {
//		std::cout << "Error " << GetLastError() << std::endl;
//		exit(-1);
//	}
//
//	BOOL b_ret;
//	MSG msg;
//	while ((b_ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
//		if (b_ret == -1) {
//			std::cout << "GetMessage returned an error" << std::endl;
//			exit(cleanup());
//		}
//		TranslateMessage(&msg);
//		DispatchMessage(&msg);
//	}
//}
//
//int main(int argc, char *argv[]) {
//	signal(SIGINT, sigint_handler);
//
//	try {
//		po::options_description desc("Allowed options");
//		desc.add_options()
//			("help", "produce help message.")
//			("record-mouse,l", "use lowLevelMouseProc to track mouse movements.")
//			;
//
//		po::variables_map vm;
//		store(parse_command_line(argc, argv, desc), vm);
//		notify(vm);
//
//		if (vm.count("help")) {
//			cout << desc << "\n";
//			return 1;
//		}
//		if (vm.count("low-level-api")) {
//			low_level_api();
//			return cleanup();
//		}
//
//		low_level_api();
//		return cleanup();
//	}
//	catch (exception &e) {
//		cerr << "error: " << e.what() << "\n";
//		exit(-1);
//	}
//	catch (...) {
//		cerr << "Exception of unknown type!\n";
//		exit(-1);
//	}
//}
//
