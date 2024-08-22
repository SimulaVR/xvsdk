#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "xv_c_wrapper.h"

double last_timestamp = 0.0;

void orientation_callback(const C_Orientation* orientation) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    
    double fps = 0.0;
    if (last_timestamp != 0.0) {
        fps = 1.0 / (orientation->hostTimestamp - last_timestamp) * 1e6;
    }
    
    printf("%04d-%02d-%02d %02d:%02d:%02d orientation@%.0ffps 3dof=(%.6f %.6f %.6f %.6f),\n",
           local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
           local->tm_hour, local->tm_min, local->tm_sec,
           fps,
           orientation->quaternion[0], orientation->quaternion[1],
           orientation->quaternion[2], orientation->quaternion[3]);
    
    last_timestamp = orientation->hostTimestamp;
}

int main() {
    printf("XVisio SDK Sample Program (C version)\n");

    const char* device_id = xv_init();
    if (device_id == NULL) {
        printf("No XVisio devices found.\n");
        return 1;
    }

    printf("Device found: %s\n", device_id);

    xv_set_orientation_callback(orientation_callback);

    printf("Orientation stream started. Press Ctrl+C to exit.\n");

    while (1) {
        usleep(100000);
    }

    xv_cleanup();

    return 0;
}

