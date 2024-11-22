#ifndef XV_C_WRAPPER_H
#define XV_C_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

  // Struct for orientation/IMU data
  typedef struct {
    double hostTimestamp;
    int64_t edgeTimestampUs;
    double quaternion[4];
  } C_Orientation;

  typedef struct {
    double hostTimestamp;
    int64_t edgeTimestampUs;
    double position[3];
    double quaternion[4];
    double confidence;
    double linearVelocity[3];
    double angularVelocity[3];
  } C_Pose;

  typedef void (*OrientationCallback)(const C_Orientation*);
  typedef void (*PoseCallback)(const C_Pose*);

  const char* xv_init_and_start_imu(OrientationCallback callback);
  const char* xv_init_and_start_slam(PoseCallback callback);

  void xv_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
