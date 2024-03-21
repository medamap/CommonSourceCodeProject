#
# Copyright (C) The Android Open Source Project
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

cmake_minimum_required(VERSION 3.4.1)

# Set the path to the Oboe library directory\
set (OBOE_DIR ../../../../oboe)
set (OBOE_BASE_DIR ../../../../../oboe)
set (CSP_SRC_DIR ../../../../../src)
set (CSP_VM_SRC_DIR ../../../../../src/vm)
set (ANDROID_SRC_DIR ./Android)

add_subdirectory(${OBOE_BASE_DIR} ./oboe-bin)
include_directories(${OBOE_DIR}/include)

# build native_app_glue as a static lib
set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

if(_SC3000)
	include(_SC3000.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_SC3000 -D_Android -D_RGB565")
elseif(_MZ700)
	include(_MZ700.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_MZ700 -D_Android -D_RGB565")
elseif(_MZ1500)
	include(_MZ1500.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_MZ1500 -D_Android -D_RGB565")
elseif(_X1)
	include(_X1.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_X1 -D_Android -D_HAS_TENKEY -D_RGB565")
elseif(_X1TWIN)
	include(_X1TWIN.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_X1TWIN -D_Android -D_HAS_TENKEY -D_RGB565")
elseif(_X1TURBO)
	include(_X1TURBO.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_X1TURBO -D_Android -D_HAS_TENKEY -D_RGB565")
elseif(_X1TURBOZ)
	include(_X1TURBOZ.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_X1TURBOZ -D_Android -D_HAS_TENKEY -D_RGB565")
elseif(_MZ80K)
	include(_MZ80K.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_MZ80K -D_Android -D_RGB565")
elseif(_MZ80B)
	include(_MZ80B.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_MZ80B -D_Android -D_RGB565")
elseif(_MZ2200)
	include(_MZ2200.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_MZ2200 -D_Android -D_HAS_TENKEY -D_RGB565")
elseif(_MSX2P)
	include(_MSX2P.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_MSX2P -D_MSX_VDP_MESS -D_Android -D_RGB565")
elseif(_PC8001)
	include(_PC8001.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_PC8001 -D_Android -D_HAS_TENKEY -D_RGB565 -Wno-narrowing")
elseif(_PC6001MK2)
	include(_PC6001MK2.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_PC6001MK2 -D_Android -D_RGB565")
elseif(_PC6001)
	include(_PC6001.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_PC6001 -D_Android -D_RGB565")

elseif(_PC8801MA)
	include(_PC8801MA.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_PC8801MA -D_Android -D_HAS_TENKEY -D_RGB565 -Wno-narrowing")
elseif(_FM7)
	include(_FM7.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_FM7 -D_Android -D_HAS_TENKEY -D_RGB565")
elseif(_FM77L4)
	include(_FM77L4.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_FM77L2 -D_Android -D_HAS_TENKEY -D_RGB565")
elseif(_PYUTA)
	include(_PYUTA.txt)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall -D_PYUTA -D_Android -D_RGB565")

endif()

find_library(log-lib log)

target_include_directories(native-activity PRIVATE
		${ANDROID_NDK}/sources/android/native_app_glue
		${OBOE_DIR}/include
		${CSP_SRC_DIR}
		${CSP_VM_SRC_DIR}
		${ANDROID_SRC_DIR}
		)

# add lib dependencies
target_link_libraries(native-activity
		android
		native_app_glue
		log
		m
		oboe
		${log-lib})

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")


