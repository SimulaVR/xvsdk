#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include "xv_c_wrapper.h"

static volatile int running = 1;

void print_timestamp() {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    printf("%04d-%02d-%02d %02d:%02d:%02d ", 
           local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
           local->tm_hour, local->tm_min, local->tm_sec);
}

void orientation_callback(const C_Orientation* orientation) {
    //compute time difference from last time this function was called and implied fps
    static double last_orientation_timestamp = 0.0;
    double fps = 0.0;

    if (last_orientation_timestamp > 0.0) {
      double dt = orientation->hostTimestamp - last_orientation_timestamp;
      if (dt > 0) fps = 1.0 / dt;
    }
    
    //print orientation quaternion with fps
    print_timestamp();
    printf("orientation@%.1ffps orientation quaternion=(%f %f %f %f)\n",
           fps,
           orientation->quaternion[0], 
           orientation->quaternion[1],
           orientation->quaternion[2], 
           orientation->quaternion[3]);
    
    //update function's static var for next iteration
    last_orientation_timestamp = orientation->hostTimestamp;
}

void pose_callback(const C_Pose* pose) {
  // Compute time difference from the last time this function was called and implied fps
  static double last_pose_timestamp = 0.0;
  double fps = 0.0;
  if (last_pose_timestamp > 0.0) {
      double dt = pose->hostTimestamp - last_pose_timestamp;
      if (dt > 0) fps = 1.0 / dt;
  }
  
  // Print timestamp, fps, and 6dof values (translation + quaternions)
  print_timestamp();
  printf("slam-get-pose p=(%f,%f,%f), Quaternion=(%f,%f,%f,%f), Confidence=%f\n",
         pose->position[0], pose->position[1], pose->position[2],
         pose->quaternion[0], pose->quaternion[1], pose->quaternion[2], pose->quaternion[3],
         pose->confidence);
  
  //Uncomment this if you want Euler angles (in degrees) printed instead:
  /*  
  // Print timestamp, fps, and 6dof values (translation + Euler angles in terms of degrees)
  double pitch = asin(2.0 * (pose->quaternion[3] * pose->quaternion[1] - pose->quaternion[2] * pose->quaternion[0])) * 180.0 / M_PI;
  double yaw = atan2(2.0 * (pose->quaternion[3] * pose->quaternion[2] + pose->quaternion[0] * pose->quaternion[1]), 
                     1.0 - 2.0 * (pose->quaternion[1] * pose->quaternion[1] + pose->quaternion[2] * pose->quaternion[2])) * 180.0 / M_PI;
  double roll = atan2(2.0 * (pose->quaternion[3] * pose->quaternion[0] + pose->quaternion[1] * pose->quaternion[2]), 
                      1.0 - 2.0 * (pose->quaternion[0] * pose->quaternion[0] + pose->quaternion[1] * pose->quaternion[1])) * 180.0 / M_PI;
  printf("slam-get-pose p=(%f,%f,%f), Euler=(%f,%f,%f), Confidence=%f\n",
         pose->position[0], pose->position[1], pose->position[2],
         pitch, yaw, roll,
         pose->confidence);
  */

  //Update state for next function call
  last_pose_timestamp = pose->hostTimestamp;
}

void sigint_handler(int sig) {
    running = 0;
}

int main() {
  signal(SIGINT, sigint_handler); //If SIGINT/C-c C-C is sent, allows us to gracefully exit without leaving the XR50 in a bind.
    
    printf("XVisio SDK Sample Program (C version)\n");
    
    printf("Initializing device and starting IMU stream...\n");

    //const char* device_id = xv_init_and_start_imu(orientation_callback);
    const char* device_id = xv_init_and_start_slam(pose_callback);

    if (device_id == NULL) {
      printf("Failed to initialize device and start IMU\n");
      return 1;
    }
    
    printf("Device initialized: %s\n", device_id);
    
    while (running) {
      usleep(100000);
    }
    
    printf("\nCleaning up...\n");
    xv_cleanup();
    printf("Done.\n");
    
    return 0;
}