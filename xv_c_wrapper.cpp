#include <cstdint>
#include <xv-sdk.h>
#include <xv-sdk-ex.h>  // Add this for DeviceEx and StereoMode
#include <cstring>
#include <chrono>
#include <thread>
#include "xv_c_wrapper.h"


extern "C" {

//Initialize global variables (ugly but for demo)
static OrientationCallback g_orientation_callback = nullptr;
static PoseCallback g_pose_callback = nullptr;
static xv::Device* g_device = nullptr;
static std::shared_ptr<xv::Slam> g_slam;
static int g_imu_callback_id = -1;
static int g_fisheye_callback_id = -1;
static std::thread g_pose_thread;
static bool g_stop_pose_thread = false;

//Pushes to our C orientation callbackfunction
void cpp_orientation_callback(const xv::Orientation& orientation) {
printf("cpp_orientation_callback\n");
    if (g_orientation_callback) {
        C_Orientation c_orientation;
        c_orientation.hostTimestamp = orientation.hostTimestamp;
        c_orientation.edgeTimestampUs = orientation.edgeTimestampUs;
        
        auto q = orientation.quaternion();
        memcpy(c_orientation.quaternion, q.data(), sizeof(double) * 4);
        
        g_orientation_callback(&c_orientation);
    }
}

//Pushes to our C 6dof callback function
void cpp_pose_polling_function(std::shared_ptr<xv::Slam> slam) {
printf("cpp_pose_polling_function\n");
    const double prediction = 0.005;
    xv::Pose pose;
    long n = 0;
    long nb_ok = 0;

    while (!g_stop_pose_thread) {
        bool ok = slam->getPose(pose, prediction);
        if (ok) {
            ++nb_ok;
            if (g_pose_callback && pose.confidence() > 0) {
                C_Pose c_pose;
                c_pose.hostTimestamp = pose.hostTimestamp();
                c_pose.edgeTimestampUs = pose.edgeTimestampUs();
                
                auto pos = pose.translation();
                memcpy(c_pose.position, pos.data(), sizeof(double) * 3);
                
                auto q = pose.quaternion();
                memcpy(c_pose.quaternion, q.data(), sizeof(double) * 4);
                
                c_pose.confidence = pose.confidence();
                
                auto linVel = pose.linearVelocity();
                memcpy(c_pose.linearVelocity, linVel.data(), sizeof(double) * 3);
                
                auto angVel = pose.angularVelocity();
                memcpy(c_pose.angularVelocity, angVel.data(), sizeof(double) * 3);
                
                g_pose_callback(&c_pose);
            }
        }
        n++;

        if (n % 1000 == 0) {
            printf("SLAM pose success rate: %.1f%% (%ld/%ld)\n", 
                   100.0 * double(nb_ok) / n, nb_ok, n);
        }
    }
}

const char* xv_init_and_start_imu(OrientationCallback callback) {
    printf("Initializing with SlamStartMode::Normal...\n");
    auto devices = xv::getDevices(10.0, "", nullptr, xv::SlamStartMode::Normal);
    if (devices.empty()) {
        printf("No devices found\n");
        return nullptr;
    }

    g_device = devices.begin()->second.get();
    printf("Device found with ID: %s\n", g_device->id().c_str());
    
    // Validate device capabilities
    if (!g_device->slam()) {
        printf("Error: SLAM not supported on this device\n");
        return nullptr;
    }

    xv::setLogLevel(xv::LogLevel(1));

    printf("Starting orientation stream...\n");
    if (!g_device->orientationStream()->start()) {
        printf("Error: Failed to start orientation stream\n");
        return nullptr;
    }

    // Register orientation callback
    g_orientation_callback = callback;
    g_imu_callback_id = g_device->orientationStream()->registerCallback(cpp_orientation_callback);

    static std::string device_id = g_device->id();
    return device_id.c_str();
}

//Includes bits from the xv_init_and_start_imu (unlike demo-api, which just crashes if you try to do this first)
const char* xv_init_and_start_slam(PoseCallback callback) {
    printf("Initializing with SlamStartMode::Normal...\n");
    auto devices = xv::getDevices(10.0, "", nullptr, xv::SlamStartMode::Normal);
    if (devices.empty()) {
        printf("No devices found\n");
        return nullptr;
    }

    g_device = devices.begin()->second.get();
    printf("Device found with ID: %s\n", g_device->id().c_str());

    // Get SLAM interface
    g_slam = g_device->slam();
    if (!g_slam) {
        printf("Error: Failed to get SLAM interface\n");
        return nullptr;
    }

    xv::setLogLevel(xv::LogLevel(1));

    //Orientation-only code which I thought might be needed
    // for this function to work (due to XR50 xrvsdk
    // internals I don't understand), but it still doesn't work in
    // monado calling this so am leaving commented out for now
    /*
    printf("Starting orientation stream...\n");
    if (!g_device->orientationStream()->start()) {
        printf("Error: Failed to start orientation stream\n");
        return nullptr;
    }
    g_orientation_callback = callback;
    g_imu_callback_id = g_device->orientationStream()->registerCallback(cpp_orientation_callback);
    static std::string device_id = g_device->id();
    return device_id.c_str();
    
    //If you use the above, you'll also need to tear it down:
    g_device->orientationStream()->unregisterCallback(g_imu_callback_id);
    g_imu_callback_id = -1;
    */
    g_fisheye_callback_id = g_device->fisheyeCameras()->registerCallback([](xv::FisheyeImages const& images) {});
    printf("Starting fisheye cameras...\n");
    if (!g_device->fisheyeCameras()->start()) {
        printf("Warning: Failed to start fisheye cameras\n");
    }

    //TODO: See if you can get pure Edge Mode to work
    if (!g_slam->start(xv::Slam::Mode::Mixed)) {
        printf("Error: Failed to start SLAM in any mode\n");
        return nullptr;
    }

    printf("Configuring SLAM...\n");

    // Give some time for initialization
    // TODO: See how low we can make this
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Setup pose callback and start pose thread
    g_pose_callback = callback;
    g_stop_pose_thread = false;
    g_pose_thread = std::thread(cpp_pose_polling_function, g_slam);

    static std::string device_id = g_device->id();
    return device_id.c_str();
}

void xv_cleanup() {
printf("xv_cleanup\n");
    // Stop pose thread if running
    if (g_pose_thread.joinable()) {
        g_stop_pose_thread = true;
        g_pose_thread.join();
    }

    // Cleanup SLAM
    if (g_slam) {
        g_slam->stop();
    }

    // Cleanup callbacks
    if (g_device) {
        if (g_device->orientationStream()) {
            g_device->orientationStream()->unregisterCallback(g_imu_callback_id);
        }
        if (g_device->fisheyeCameras()) {
            g_device->fisheyeCameras()->unregisterCallback(g_fisheye_callback_id);
        }
    }

    // Clean up ugly global state vars
    g_device = nullptr;
    g_slam.reset();
    g_orientation_callback = nullptr;
    g_pose_callback = nullptr;
}

}

