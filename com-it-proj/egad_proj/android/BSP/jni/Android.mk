# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDE_DIRS  := $(LOCAL_PATH)/Calypso
LOCAL_C_INCLUDES := $(LOCAL_PATH)/Calypso
LOCAL_EXPORT_CFLAGS := -DENABLE_COMM
LOCAL_MODULE    := bsp-jni
LOCAL_SRC_FILES := bsp_api.c bsp_jni.c lib_bmp.c os_porting.c printer_bmp.c sii_api.c  BSPutil.c gpio_util.c GPSDriver.c Calypso/LibTime.c Calypso/LibCrypto.c Calypso/LibC.c Calypso/7816_stub.c Calypso/Bit2Byte.c Calypso/ClyAppApi.c Calypso/ClyApp.c Calypso/ClyCrdOs.c Calypso/ClyKey.c Calypso/ClySamOs.c Calypso/ClySessn.c Calypso/ClyTktOs.c Calypso/Core.c    
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -landroid 
include $(BUILD_SHARED_LIBRARY)
