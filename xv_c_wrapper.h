#ifndef XV_C_WRAPPER_H
#define XV_C_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    double hostTimestamp;
    int64_t edgeTimestampUs;
    double quaternion[4];
} C_Orientation;

typedef void (*OrientationCallback)(const C_Orientation*);

const char* xv_init();
void xv_set_orientation_callback(OrientationCallback callback);
void xv_cleanup();

#ifdef __cplusplus
}
#endif

#endif // XV_C_WRAPPER_H
