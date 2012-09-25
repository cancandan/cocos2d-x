LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := purelua_shared

LOCAL_MODULE_FILENAME := libpurelua

LOCAL_SRC_FILES := 	lua/lapi.c \
                  	lua/lauxlib.c \
          			lua/lbaselib.c \
          			lua/lcode.c \
          			lua/ldblib.c \
          			lua/ldebug.c \
          			lua/ldo.c \
          			lua/ldump.c \
          			lua/lfunc.c \
          			lua/lgc.c \
          			lua/linit.c \
          			lua/liolib.c \
          			lua/llex.c \
          			lua/lmathlib.c \
          			lua/lmem.c \
          			lua/loadlib.c \
          			lua/lobject.c \
          			lua/lopcodes.c \
          			lua/loslib.c \
          			lua/lparser.c \
          			lua/lstate.c \
          			lua/lstring.c \
          			lua/lstrlib.c \
          			lua/ltable.c \
          			lua/ltablib.c \
          			lua/ltm.c \
          			lua/lua.c \
          			lua/lundump.c \
          			lua/lvm.c \
          			lua/lzio.c \
          			lua/print.c \
					engine/main.cpp \
					engine/LuaEngine.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/engine \
					$(LOCAL_PATH)/lua

LOCAL_LDLIBS :=  -llog

include $(BUILD_SHARED_LIBRARY)
