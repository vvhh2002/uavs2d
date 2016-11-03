### This file is jni/build-android/Android.mk

LOCAL_PATH:=$(call my-dir)
SRC_PATH:=../../source
-include android-ndk-profiler.mk 
include $(CLEAR_VARS)

#LOCAL_CFLAGS := -pg 
#LOCAL_STATIC_LIBRARIES := andprof

LOCAL_LDLIBS:=-L$(SYSROOT)/usr/lib -llog
#LOCAL_LDLIBS    += -llog

### Name of the local module 
LOCAL_MODULE    :=uavs

### for posix pthread
LOCAL_SHARED_LIBRARIES := libcutil

### include search path when compiling all sources (C,C++,Assembly)
LOCAL_C_INCLUDES +=platforms/android-18/arch-arm/usr/include   \
                   $(LOCAL_PATH)                               \
                   $(LOCAL_PATH)/../../source/decore            

### The files that make up the source code
LOCAL_SRC_FILES :=$(SRC_PATH)/decoder/AEC.c                  \
                  $(SRC_PATH)/decoder/block.c              \
                  $(SRC_PATH)/decoder/codingUnit.c               \
                  $(SRC_PATH)/decoder/DecAdaptiveLoopFilter.c               \
                  $(SRC_PATH)/decoder/header.c          \
                  $(SRC_PATH)/decoder/image.c           \
                  $(SRC_PATH)/decoder/loopFilter.c                 \
                  $(SRC_PATH)/decoder/wquant.c              \
                  $(SRC_PATH)/decoder/uavs2d.c                  \
                  $(SRC_PATH)/decore/biaridecod.c                \
				  $(SRC_PATH)/decore/bitstream.c              \
		          $(SRC_PATH)/decore/block_info.c            \
		          $(SRC_PATH)/decore/ComAdaptiveLoopFilter.c             \
		          $(SRC_PATH)/decore/common_tables.c           \
		          $(SRC_PATH)/decore/common.c            \
				  $(SRC_PATH)/decore/inter-prediction.c           \
				  $(SRC_PATH)/decore/intra-prediction.c              \
				  $(SRC_PATH)/decore/loop-filter.c          \
				  $(SRC_PATH)/decore/pixel.c         \
				  $(SRC_PATH)/../utest.c         \
				  $(SRC_PATH)/decore/threadpool.c      \
				  $(SRC_PATH)/decore/transform.c        \
				  $(SRC_PATH)/decore/neon/android_sao_neon.s		\
				  $(SRC_PATH)/decore/neon/android_sao.c		\
				  $(SRC_PATH)/decore/neon/android.c    \
				  $(SRC_PATH)/decore/neon/android_pixel.s    \
				  $(SRC_PATH)/decore/neon/asm.s               \
				  $(SRC_PATH)/decore/neon/android_inter_pred.s \
				  $(SRC_PATH)/decore/neon/android_idct.s        \
				  $(SRC_PATH)/decore/neon/android_alf.s			\
				  $(SRC_PATH)/decore/neon/android_deblock.s    \
				  $(SRC_PATH)/decore/neon/android_intra_pred.s

LOCAL_CFLAGS    += -DHAVE_NEON=1
LOCAL_CFLAGS 	+= -mfpu=neon

#LOCAL_CFLAGS += -std=gnu99

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)
