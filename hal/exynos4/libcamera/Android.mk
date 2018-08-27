LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# HAL module implemenation stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.product.board>.so
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include  \
			system/media/camera/inlcude  \
			system/core/inlcude 

LOCAL_SRC_FILES:= \
	SecCamera.cpp SecCameraHWInterface.cpp

LOCAL_SHARED_LIBRARIES:=libcamera_metadata libutils libcutils libbinder liblog libcamera_client libhardware  libhwjpeg

ifeq ($(TARGET_SOC), exynos4210)
LOCAL_SHARED_LIBRARIES += libs5pjpeg
LOCAL_CFLAGS += -DSAMSUNG_EXYNOS4210
endif

#ifeq ($(TARGET_SOC), exynos4x12)
LOCAL_SHARED_LIBRARIES += libhwjpeg
LOCAL_CFLAGS += -DSAMSUNG_EXYNOS4x12
#endif

ifeq ($(BOARD_USE_V4L2), true)
LOCAL_CFLAGS += -DBOARD_USE_V4L2
endif

ifeq ($(BOARD_USE_V4L2_ION), true)
LOCAL_CFLAGS += -DBOARD_USE_V4L2
LOCAL_CFLAGS += -DBOARD_USE_V4L2_ION
endif


#include $(BUILD_SHARED_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE := camera.$(TARGET_DEVICE)

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
