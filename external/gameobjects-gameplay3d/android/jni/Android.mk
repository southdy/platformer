
LOCAL_PATH := $(call my-dir)/../../src

include $(CLEAR_VARS)
LOCAL_MODULE    := libgameobjects
LOCAL_SRC_FILES := \
    Component.cpp \
    GameObject.cpp \
    GameObjectController.cpp \
    GameObjectMessageFactory.cpp

LOCAL_CFLAGS := -D__ANDROID__ -DGP_SCENE_VISIT_EXTENSIONS -I "../../../external/GamePlay/gameplay/src" -I"../../GamePlay/external-deps/include"
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/native_app_glue)
