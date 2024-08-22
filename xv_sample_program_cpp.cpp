#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <xv-sdk.h>

double lastTimestamp = 0.0;

void orientationCallback(const xv::Orientation& orientation) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") 
              << " orientation@" 
              << std::fixed << std::setprecision(0) << 1.0 / (orientation.hostTimestamp - lastTimestamp) * 1e6 << "fps "
              << "3dof=(" 
              << std::setprecision(6)
              << orientation.quaternion()[0] << " "
              << orientation.quaternion()[1] << " "
              << orientation.quaternion()[2] << " "
              << orientation.quaternion()[3] << ")," << std::endl;
    lastTimestamp = orientation.hostTimestamp;
}

int main() {
    std::cout << "XVisio SDK Sample Program (C++ version)" << std::endl;

    auto devices = xv::getDevices();
    if (devices.empty()) {
        std::cerr << "No XVisio devices found." << std::endl;
        return 1;
    }

    auto device = devices.begin()->second;
    std::cout << "Device found: " << device->id() << std::endl;

    auto orientationStream = device->orientationStream();
    if (!orientationStream) {
        std::cerr << "Orientation stream not supported on this device." << std::endl;
        return 1;
    }

    if (!orientationStream->start()) {
        std::cerr << "Failed to start orientation stream." << std::endl;
        return 1;
    }

    orientationStream->registerCallback(orientationCallback);

    std::cout << "Orientation stream started. Press Ctrl+C to exit." << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
