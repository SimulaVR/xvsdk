# Xvisio SDK Guide

**Version 1.2**  
**Date**: 2022.03  
**Confidential**

## Copyright and Proprietary Information Notice
**Copyright © 2018 Xvisio Ltd. All rights reserved.**  
This document contains confidential and proprietary information that is the property of Xvisio Ltd. All other product or company names may be trademarks of their respective owners.

**Xvisio Ltd.**  
Room 408, building 1, No.288, Tong Xie Road, Changning District, Shanghai  
http://www.Xvisiotech.com

## Revision History
| Version | Description     | Author |
|---------|-----------------|--------|
| 1.0     | First version   | Xvisio |
| 1.1     | XVSDK 3.0.2     | Xvisio |
| 1.2     | XVSDK 3.1.0     | Xvisio |

## Contents
1. Overview
2. Environment Construction for Xvisio SDK
   - 2.1 Android SDK Environment Construction
     - 2.1.1 Android SDK Structure
     - 2.1.2 Android Box Requirements
     - 2.1.3 Android Environment Test Verify
   - 2.2 Windows SDK Environment Construction
   - 2.3 Ubuntu SDK
3. Xvisio SDK Development
   - 3.1 Xvisio Slam
   - 3.2 VIO Mode
   - 3.3 CSLAM Mode
   - 3.4 3DOF (Rotation)
   - 3.5 Plane Detection (TOF/stereo)
   - 3.6 Map Sharing
   - 3.7 HID interfaces
   - 3.8 Calibration Parameter API
   - 3.9 HOT Plug
   - 3.10 Get Fisheye Data
   - 3.11 Get RGB Data
   - 3.12 Get TOF Data
   - 3.13 Get RGBD Data
   - 3.14 CNN
   - 3.15 Gesture Function
   - 3.16 Get SGBM Data
   - 3.17 Python-wrapper

## 1. Overview
This document serves as a user guide for the Xvisio SDK, designed to help developers understand the SDK and quickly develop related applications (SLAM, camera, audio, etc.). The document is divided into two main parts: SDK environment construction and sample code introduction.

Currently, the Xvisio SDK supports three platforms: Android, Ubuntu, and Windows. The main difference between these platforms is that the `.so` is compiled separately, but the API remains the same. Section 2 introduces how to install and use the Xvisio SDK on these platforms. Section 3 describes how to call the SDK API to achieve different functions across all platforms.

## 2. Environment Construction for Xvisio SDK
This chapter sequentially introduces how to install and use Xvisio SDK on Android, Ubuntu, and Windows platforms. Relevant precautions are also included.

### 2.1 Android SDK Environment Construction

#### 2.1.1 Android SDK Structure
The Android SDK structure is shown below:

- The folder `Android` contains the makefile compiled by NDK. If the user modifies the demo code in the samples folder, they can use `ndk-build -j 5` to compile and generate the corresponding executable file. The compiled file path is in `/Android/libs/arm64-v8a` (armeabi-v7a).
- File `bin` is the tool of 64bit and 32bit.
- File `doc` contains the definition file of class and interfaces.
- Folder `examples` is our demo code, mainly describing how to use our SDK API.
- The `include` folder is the header file of the SDK API.
- The folder `libs` contains `SDK so` files. `arm64-v8a` is a 64bit library, and `armeabi-v7a` is a 32bit library.
- `deploy.sh` is a push script file. Executing `./deploy-arm64-v8a.sh` will push the 64bit library to the Android platform; executing `./deploy-armeabi-v7a.sh` will push the 32bit library to the Android platform. Modify the `libdir` in `deploy.sh` to modify the push lib path.

#### 2.1.2 Android Box Requirements
- The Android OS version should be above (include) 7.
- Developers need to confirm that the Android kernel supports HID and UVC modules. If not, the kernel needs to be recompiled, and the HID & UVC modules should be added.
- Android OS is a userdebug version.
- If only SLAM function is needed, USB 2.0 can satisfy the demand. For complete functions (SLAM, RGB, TOF, audio), USB 3.0 is needed.

#### 2.1.3 Android Environment Test Verify
To test and verify the Xvisio Android SDK, connect the box with a PC after powering it on and execute the following commands:

```

adb root adb remount cd android_xxx_sdk adb push libs /data/test/ adb push bin /data/test/

```lua

After connecting, execute:

```

lsusb (make sure the device can be found. If the terminal shows `f408`, it means the device has been successfully connected with the box.) adb shell cd /data/test/bin/arm64-v8a/ LD_LIBRARY_PATH=../../libs/arm64-v8a/ ./demo-api

```sql

Open a new terminal and execute:

```

adb shell cd /data/test/bin/arm64-v8a/ LD_LIBRARY_PATH=../../libs/arm64-v8a/ ./pipe_srv

```sql

If the 3DOF, 6DOF, and other sensor data can be obtained normally, it means there is no problem with the Android environment, and subsequent developers can integrate the SDK into the Android system.

### 2.2 Windows SDK Environment Construction

#### 2.2.1 XVSDK Installation
Select the installation file according to the PC configuration. Double-click the `.exe` file and follow the prompts to complete the installation.

#### 2.2.2 Driver Installation
1. After connecting the device with the PC, open the Windows Device Manager.
2. Check the USB port.
3. Download the tool "Zadig" from [https://zadig.akeo.ie/downloads/zadig-2.4.exe](https://zadig.akeo.ie/downloads/zadig-2.4.exe). Double-click "Zadig", select "VSC interface" and "libusb-win32", then click "Install Driver".

If the data and image can be retrieved successfully, the installation is successful.

### 2.3 Ubuntu SDK

#### 2.3.1 Ubuntu SDK Installation

Supported PC versions:
- Ubuntu 16.04 'Xenial'
- Ubuntu 18.04 'Bionic'
- Ubuntu 20.04 'Focal'

Install xvsdk by `.deb`:

```bash
sudo apt-get update
sudo apt-get install -y g++ cmake libjpeg-dev zlib1g-dev udev libopencv-core3.2 libopencv-highgui-dev liboctomap1.8 libboost-chrono-dev libboost-thread-dev libboost-filesystem-dev libboost-system-dev libboost-program-options-dev libboost-date-time-dev
sudo dpkg -i xvsdk_3.2.0-20220304_bionic_amd64.deb
```

The file is saved in `/usr/` after the installation finishes. Use the `whereis` command to find the file position.

3. Xvisio SDK Development
-------------------------

This chapter introduces how to develop SLAM, CSLAM, plane detection, 3DOF, and other functions based on the Xvisio SDK. It also explains how to obtain camera data such as RGB, TOF, eyetracking, and how to integrate audio functions. Specific example code can be found in `examples\xslam-edge-plus-SDK\demo\demo.cpp`.

### 3.1 Xvisio Slam

#### 3.1.1. Slam Mode Settings

Xvisio slam supports two modes: mix mode (part of the SLAM algorithm runs on the device side, and some on the host side) and edge mode (the SLAM algorithm runs entirely on the device side).

```cpp
// Enable edge mode
device->slam()->start(xv::Slam::Mode::Edge);

// Enable mix mode
device->slam()->start(xv::Slam::Mode::Mixed);
```

#### 3.1.2. Slam Center Point

The device's 6DOF center point is on IMU (6DOF's xyz means IMU's translation; pitch/yaw/roll means IMU's rotation). After starting SLAM, a coordinate system is established based on the device's gravity direction. The origin of the coordinate system is on IMU when SLAM starts (the xyz value is 0 when SLAM starts; pitch/yaw/roll represent the rotation of IMU/device at that time).

#### 3.1.3. 6DOF Structure

6DOF data supports 3 formats (Eulerian angle, rotation matrix, quaternion) to express rotation. The 6DOF structure is as follows:

```cpp
struct Pose : public details::PosePred_<double> {
    Pose();
    Pose(Vector3d const& translation, Matrix3d const& rotation,
         double hostTimestamp = std::numeric_limits<double>::infinity(), 
         std::int64_t edgeTimestamp = (std::numeric_limits<std::int64_t>::min)(), 
         double c=0.);
    Pose prediction(double dt) const;
};
```

The time base of `hostTimestamp` is the host side, starting with the host side boot (starting timing from 0). The time base of `deviceTimestamp` is the device side (glass or module), starting with device boot (starting timing from 0). Confidence is the trustworthy level of 6DOF, and value 0 means SLAM lost.

#### 3.1.4. Get 6DOF

Mix mode supports two ways to obtain 6DOF data, one is callback, and the other is active acquisition. The edge mode currently only supports the callback method to obtain 6DOF.

1. Callback

```cpp
device->slam()->registerCallback(poseCallback);
```

The frame rate of the callback is generally determined by the frame rate of the IMU (6DOF uses IMU for fusion); if the user wants to get 6DOF data with prediction from the callback, just set the `filter_prediction` value in the `.ini` file.

2. Active acquisition

```cpp
bool getPose(Pose &pose, double prediction);
```

Developers can call this interface to actively obtain 6DOF data and set the prediction time at the same time, but it is recommended to be less than 0.016 (16ms). If the prediction is too large, 6DOF prediction may not be accurate.

### 3.2 VIO Mode

The following interfaces are mainly used to realize the VIO function:

```cpp
bool start();
bool stop();
int registerCallback(std::function<void (xv::Pose const&)>);
bool unregisterCallback(int callbackId);
bool getPose(Pose &pose, double prediction);
```

VIO mode does not include map loop closure. As the odometer increases, the cumulative error will increase. In normal use, it is generally recommended that users use CSLAM mode, first build a map, and then run SLAM. The following example shows how to pass the above interface to run SLAM:

1. Register lost callback.
2. Call `start()` to turn on SLAM.
3. Call `getPose()` to get 6DOF.
4. Call `stop()` to stop SLAM.

For specific code, refer to `demo.cpp` (case 2, case 3).

### 3.3 CSLAM Mode

The following interfaces are mainly used to achieve CSLAM function:

```cpp
bool start();
bool stop();
bool loadMapAndSwitchToCslam(std::streambuf &mapStream, std::function<void(int)> done_callback, std::function<void(float)> localized_on_reference_map);
bool saveMapAndSwitchToCslam(std::streambuf &mapStream, std::function<void(int, int)> done_callback, std::function<void(float)> localized_on_reference_map);
```

CSLAM steps:

1. To build a good map, ensure that the map contains all the viewpoints required by the application.
2. Ensure good overlap between different recording viewpoints by walking twice on the same path.
3. Avoid moving fast or facing areas with no features during the recording process.

For specific code, refer to `demo-api.cpp` (case 6, case 7). The API call process for switching to CSLAM after mapping is as follows:

1. Call `start()` and register 6DOF callback.
2. Mapping.
3. After mapping, call `saveMapAndSwitchToCslam` to save the map and switch to CSLAM.
4. Call `stop()` to stop CSLAM.

### 3.4 3DOF (Rotation)

If the application only needs 3DOF data, the SDK can get IMU data and then convert it to 3DOF. The 3DOF structure and interface are as follows:

```cpp
class Orientation {
    Matrix3d m_rotation;
    Vector4d m_quaternions; //!< [qx,qy,qz,qw]
    Vector3D m_angularVelocity = Vector3D{0.,0.,0.};
    Vector3D m_angularAcceleration = Vector3D{0.,0.,0.};
public:
    double hostTimestamp = std::numeric_limits<double>::infinity();
    std::int64_t edgeTimestampUs = (std::numeric_limits<std::int64_t>::min)();
    Vector4d const& quaternion() const;
    Orientation prediction(double dt) const;
};
int registerCallback(std::function<void(Orientation const &)>);
bool unregisterCallback(int callbackID);
```

For specific code, refer to `demo-api.cpp` (case 1, case 2).

### 3.5 Plane Detection (TOF/stereo)

The interface of Plane detection is shown below:

```cpp
// stereo plane detection API
int registerStereoPlanesCallback(std::function<void(std::shared_ptr<const std::vector<xv::Plane>>)> planeCallback);
bool unregisterStereoPlanesCallback(int callbackID);

// tof plane detection API
int registerTofPlanesCallback(std::function<void(std::shared_ptr<const std::vector<Plane>>)> planeCallback);
bool unregisterTofPlanesCallback(int callbackID);
```

Plane detection needs to work with SLAM, and the position of the plane is based on the SLAM coordinate system. `std::vector<Vector3d> points` represents the 3D feature points of the plane. Connecting these 3D features gives the real detected plane area.

For specific code, refer to `demo-api.cpp` (case 13 and case 14) for TOF plane detection and `demo-api.cpp` (case 23 and case 24) for stereo plane detection.

### 3.6 Map Sharing

The interface of Map sharing is as follows:

```cpp
bool start();
bool stop();
bool loadMapAndSwitchToCslam(std::streambuf &, std::function<void (int)>, std::function<void (float)>);
```

Map sharing is an advanced function of CSLAM, allowing multiple people to interact in real-time in the same scene. For specific code, refer to `demo-api.cpp` (case 19 and case 20) for mapping code, and `mapdemo-api.cpp` (case 21 and case 22) for usage code.

### 3.7 HID Interfaces

Many control commands of the device are based on HID channels. The read and write interfaces of HID are shown below:

```cpp
bool hidWriteAndRead(const std::vector<unsigned char> &command, std::vector<unsigned char> &result);
```

For specific code, refer to the example:

```cpp
m_deviceDriver->device()->hidWriteAndRead({0x02, 0xfe, 0x20, 0x0D}, result);
```

### 3.8 Calibration Parameter API

The SDK provides a calibration parameter API for fisheye, RGB, TOF, and display. The external coordinate system (right-hand) of all cameras is based on IMU as the origin. Fisheye internal parameters support two types: Polynomial Distortion Model (RGB, TOF) and Unified camera model (fisheye).

The interface is as follows:

```cpp
virtual const std::vector<Calibration>& calibration();
```

For specific code, refer to `demo-api.cpp` (case 37) for fisheye calibration parameter, and `demo-api.cpp` (case 31) for RGB calibration parameter.

### 3.9 HOT Plug

The interface for Hotplug is:

```cpp
int registerPlugEventCallback(const std::function<void(std::shared_ptr<Device> device, PlugEventType type)> &Callback);
```

### 3.10 Get Fisheye Data

Fisheye frequency:

```cpp
enum class ResolutionMode {
    LOW = 1, ///< Low resolution (typically QVGA)
    MEDIUM = 2, ///< Medium resolution (typically VGA)
    HIGH = 3 ///< High resolution (typically HD 720)
};
```

Fisheye interfaces:

```cpp
virtual int registerCallback(std::function<void(T)>) = 0;
virtual bool unregisterCallback(int callbackId) = 0;
```

### 3.11 Get RGB Data

The interface for RGB is as follows:

```cpp
virtual int registerCallback(std::function<void(T)>) = 0;
virtual bool unregisterCallback(int callbackId) = 0;
```

For specific code, refer to `demo-api.cpp` (case 9) for getting RGB data.

### 3.12 Get TOF Data

TOF mode introduction:

Frequency: single frequency (SF) | double frequency (DF)  
Mode: IQ | M2 | edge  
Frequency: 5-30 FPS

Different modes correspond to different FPS. The interface for TOF is as follows:

```cpp
virtual int registerCallback(std::function<void(T)>) = 0;
virtual bool unregisterCallback(int callbackId) = 0;
virtual bool setStreamMode(StreamMode mode);
virtual bool setDistanceMode(DistanceMode mode);
enum class StreamMode { DepthOnly = 0, CloudOnly, DepthAndCloud, None };
enum class DistanceMode { Short = 0, Middle, Long };
```

For specific code, refer to `demo-api` (case 11, case 39, case 40) for getting RGB data.

### 3.13 Get RGBD Data

The interfaces for RGBD are:

```cpp
int registerColorDepthImageCallback(std::function<void(const DepthColorImage&)>);
virtual bool unregisterColorDepthImage(int callbackId);
struct DepthColorImage {
    std::size_t width = 0; //!< width of the image (in pixel)
    std::size_t height = 0; //!< height of the image (in pixel)
    std::shared_ptr<const std::uint8_t> data = nullptr; //! RGBD = RGB + Depth Map.
    image data of RGB-D pixels : RGB (3 bytes) D (float 4bytes)
    double hostTimestamp = std::numeric_limits<double>::infinity();
};
```

For specific code, note that both `enable colorCamera` and `TOFCamers` need to be enabled to get RGBD data. Refer to `demo-api.cpp` (case 9) to get RGB data.

### 3.14 CNN

#### 3.14.1 CNN Interfaces:

```cpp
virtual bool setDescriptor(const std::string &filepath) = 0;
virtual bool setModel(const std::string &filepath) = 0;
virtual bool setSource(const Source &source) = 0;
virtual xv::ObjectDetector::Source getSource() const = 0;
virtual xv::ObjectDescriptor getDescriptor() const = 0;
enum class Source { LEFT = 0, RIGHT, RGB, TOF };
```

For specific code, refer to `demo-api.cpp` (case 42) for CNN function.

### 3.15 Gesture Function

#### 3.15.1 MNN

Push the MNN model file (including `hand_landmark.mnn`, `palm_detection_pb.mnn`) to the default path `/etc/xvisio/gesture/`. A new MNN model file path can be set by `setConfigPath` after pushing the model file.

21 feature points of gesture:

#### 3.15.2 Interfaces of Gesture:

```cpp
struct keypoint {
    float x = -1;
    float y = -1;
    float z = -1;
};
struct GestureData {
    int index[2] = {-1,-1}; // gesture index value, left hand first and then right hand
    keypoint position[2]; // position of gesture, left hand first and then right hand.
    double hostTimestamp = std::numeric_limits<double>::infinity(); // start timing (0) from host booting.
    std::int64_t edgeTimestampUs = std::numeric_limits<std::int64_t>::min(); // start timing (0) from device booting
    float distance; // reserved to keep the moving distance of dynamic gesture.
    float confidence; // reserved to keep confidence.
};
virtual void setConfigPath(std::string config) = 0; // set path of MNN configuration file
virtual int registerDynamicGestureCallback(std::function<void(GestureData const &)>) = 0; // register dynamic gesture callback, get index value of dynamic gesture.
virtual bool UnregisterDynamicGestureCallback(int callbackID) = 0; // unregister dynamic gesture callback
virtual int registerKeypointsCallback(std::function<void(std::shared_ptr<const std::vector<keypoint>>)> callback) = 0; // register gesture 21Dof callback based on RGB coordinate system and 2D coordinate.
virtual bool unregisterKeypointsCallback(int callbackId) = 0; // unregister gesture 21Dof callback
virtual int registerSlamKeypointsCallback(std::function<void(std::shared_ptr<const std::vector<keypoint>>)> callback) = 0; // reserved to get gesture 21Dof & 3D coordinate value based on SLAM coordinate system.
virtual bool unregisterSlamKeypointsCallback(int callbackId) = 0; // reserved to cancel the callback based on SLAM coordinate system.
```

For specific code, refer to `demo-api.cpp` (case 43-47) for test items.

### 3.16 Get SGBM Data

SGBM interfaces:

```cpp
virtual int registerCallback(std::function<void(T)>) = 0;
virtual bool unregisterCallback(int callbackId) = 0;
virtual Mode mode() const = 0; // set SGBM mode interface, 0-Hardware, 1-Software
virtual bool start(const std::string &sgbmConfig) = 0;
virtual bool start(const sgbm_config &sgbmConfig) = 0;
virtual bool setConfig(const std::string &sgbmConfig) = 0;
```

SGBM default start configuration information:

```cpp
static struct xv::sgbm_config global_config = {
    0, // enable_dewarp
    3.5, // dewarp_zoom_factor
    1, // enable_disparity
    1, // enable_depth
    0, // enable_point_cloud
    0.11285, // baseline
    69, // fov
    255, // disparity_confidence_threshold
    {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, // homography
    0, // enable_gamma
    2.2, // gamma_value
    0, // enable_gaussian
    0, // mode
    5000, // max_distance
    100, // min_distance
};
```

For specific code, get SGBM data from `demo-api.cpp` (case 58, case 59).

### 3.17 Python-wrapper

The SDK provides a Python interface to get images or parameter data of SLAM, IMU, fisheye, RGB, TOF, SGBM, etc. Python-wrapper only supports Windows OS.

1. Install Windows xvsdk and select "python-wrapper".
2. The installation path `\bin\python-wrapper` contains `PythonDemo.py`.
3. Connect the device with the PC after installing Python 3.9 and use `cmd` to run Python.

