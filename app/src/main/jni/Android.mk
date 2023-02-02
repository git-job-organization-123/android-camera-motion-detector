LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCVROOT:=C:/OpenCV-android-sdk
OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED
include ${OPENCVROOT}/sdk/native/jni/OpenCV.mk

# Set the name of the library
LOCAL_MODULE := motiondetector
LOCAL_LDLIBS += -lGLESv3

include $(BUILD_SHARED_LIBRARY)
