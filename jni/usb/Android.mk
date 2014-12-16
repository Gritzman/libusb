LOCAL_PATH := $(call my-dir)  
APP_PLATFORM := android-16

include $(CLEAR_VARS)  


LOCAL_SRC_FILES:= \
	 core.c \
	 descriptor.c \
	 io.c \
	 sync.c \
	 os/linux_usbfs.c \
	 os/threads_posix.c \
	 jinterface.c
			
LOCAL_LDLIBS := -llog

LOCAL_MODULE := usbapp 


include $(BUILD_SHARED_LIBRARY)  
