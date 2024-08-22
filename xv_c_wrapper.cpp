#include <cstdint>
#include <xv-sdk.h>
#include <cstring>

extern "C" {

typedef struct {
    double hostTimestamp;
    int64_t edgeTimestampUs;
    double quaternion[4];
} C_Orientation;

typedef void (*OrientationCallback)(const C_Orientation*);

OrientationCallback g_callback = nullptr;

void cpp_orientation_callback(const xv::Orientation& orientation) {
    if (g_callback) {
        C_Orientation c_orientation;
        c_orientation.hostTimestamp = orientation.hostTimestamp;
        c_orientation.edgeTimestampUs = orientation.edgeTimestampUs;
        auto q = orientation.quaternion();
        memcpy(c_orientation.quaternion, q.data(), sizeof(double) * 4);
        g_callback(&c_orientation);
    }
}

xv::Device* g_device = nullptr;
std::shared_ptr<xv::OrientationStream> g_orientation_stream;

const char* xv_init() {
    auto devices = xv::getDevices();
    if (devices.empty()) {
        return nullptr;
    }
    g_device = devices.begin()->second.get();
    return g_device->id().c_str();
}

void xv_set_orientation_callback(OrientationCallback callback) {
    g_callback = callback;
    if (g_device) {
        g_orientation_stream = g_device->orientationStream();
        if (g_orientation_stream) {
            g_orientation_stream->start();
            g_orientation_stream->registerCallback(cpp_orientation_callback);
        }
    }
}

void xv_cleanup() {
    if (g_orientation_stream) {
        g_orientation_stream->stop();
    }
    g_device = nullptr;
    g_orientation_stream.reset();
}

}
