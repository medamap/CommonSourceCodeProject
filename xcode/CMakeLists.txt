cmake_minimum_required(VERSION 3.15)
project(CSCP_iPadOS LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

# Appleプラットフォーム前提
# macOS 10.13 High Sierra以降に対応（Metal 2.0対応）
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13")

# 機種名の設定（コマンドラインから指定可能）
if(NOT DEFINED MACHINE)
    set(MACHINE "x1turbo" CACHE STRING "Target machine name (lowercase)")
endif()

# マシン名を大文字に変換（設定ファイル名用）
string(TOUPPER ${MACHINE} MACHINE_UPPER)

# ビルドターゲットのベースパス
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/..)
set(SRC_DIR ${PROJECT_ROOT}/src)
set(OSD_DIR ${SRC_DIR}/Xcode)
set(MACHINE_DIR ${CMAKE_SOURCE_DIR}/Machines)
set(MACHINE_NAME "_${MACHINE_UPPER}")

message(STATUS "Building for machine: ${MACHINE} (config: ${MACHINE_NAME})")

# includeパス
include_directories(
    ${SRC_DIR}
    ${OSD_DIR}
)

# 機種別ファイルリストを読み込む
if(EXISTS ${MACHINE_DIR}/${MACHINE_NAME}.txt)
    include(${MACHINE_DIR}/${MACHINE_NAME}.txt)
else()
    message(FATAL_ERROR "Machine configuration not found: ${MACHINE_DIR}/${MACHINE_NAME}.txt")
endif()

# ライブラリとしてビルド（Swiftは除外）
add_library(cscp STATIC
     ${SOURCES}
 )

# プリプロセッサ定義
target_compile_definitions(cscp PRIVATE
    -D_IOS -D_XCODE
    -D${MACHINE_NAME}
    -DCONFIG_NAME_STR="${MACHINE}"
    -DUSE_GUI
)

# 機種別の追加定義（Android版と同等の設定）
# Medamap and Claude: Android版の機種別定義を正確に移植

# 基本的にすべての機種で使用される定義
target_compile_definitions(cscp PRIVATE
    -D_RGB565
    -D_EXTEND_MENU
)

# 機種固有の定義
if(MACHINE STREQUAL "x1" OR MACHINE STREQUAL "x1turbo" OR MACHINE STREQUAL "x1turboz" OR MACHINE STREQUAL "x1twin")
    target_compile_definitions(cscp PRIVATE -D_HAS_TENKEY -DUSE_TV_CONTROL)
elseif(MACHINE STREQUAL "mz80k" OR MACHINE STREQUAL "mz80a" OR MACHINE STREQUAL "mz80b" OR 
       MACHINE STREQUAL "mz700" OR MACHINE STREQUAL "mz800" OR MACHINE STREQUAL "mz1500")
    # MZ80K/80A/80B/700/800/1500 は追加定義なし（基本定義のみ）
elseif(MACHINE STREQUAL "mz2200" OR MACHINE STREQUAL "mz2500" OR MACHINE STREQUAL "mz2800" OR
       MACHINE STREQUAL "mz3500" OR MACHINE STREQUAL "mz5500" OR MACHINE STREQUAL "mz6500" OR MACHINE STREQUAL "mz6550")
    target_compile_definitions(cscp PRIVATE -D_HAS_TENKEY)
elseif(MACHINE STREQUAL "msx1" OR MACHINE STREQUAL "msx2" OR MACHINE STREQUAL "msx2p")
    target_compile_definitions(cscp PRIVATE -D_MSX_VDP_MESS)
elseif(MACHINE STREQUAL "pc8801" OR MACHINE STREQUAL "pc8801ma" OR MACHINE STREQUAL "pc8801mk2")
    target_compile_definitions(cscp PRIVATE -D_HAS_TENKEY)
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-Wno-narrowing")
elseif(MACHINE STREQUAL "pc9801" OR MACHINE STREQUAL "pc9801e" OR MACHINE STREQUAL "pc9801ra" OR
       MACHINE STREQUAL "pc9801u" OR MACHINE STREQUAL "pc9801vf" OR MACHINE STREQUAL "pc9801vm" OR MACHINE STREQUAL "pc9801vx")
    target_compile_definitions(cscp PRIVATE -D_HAS_TENKEY)
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-Wno-narrowing")
elseif(MACHINE STREQUAL "bubcom80")
    target_compile_definitions(cscp PRIVATE -D_HAS_TENKEY)
elseif(MACHINE STREQUAL "pcengine")
    target_compile_definitions(cscp PRIVATE -D_HAS_TENKEY)
    set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-Wno-narrowing")
endif()

# GUI版の設定（macOSの場合はデフォルトでGUI版）
if(NOT DEFINED USE_GUI)
    if(NOT IOS)
        set(USE_GUI ON)
    else()
        set(USE_GUI OFF)
    endif()
endif()

# 実行ファイル出力ディレクトリをbin/に設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 実行可能ターゲット（機種名を使用）
add_executable(${MACHINE} ${SRC_DIR}/Xcode/xcode_main.mm)

# 実行ファイルにもGUI定義を追加
target_compile_definitions(${MACHINE} PRIVATE
    -D_IOS -D_XCODE
    -D${MACHINE_NAME}
    -DCONFIG_NAME_STR="${MACHINE}"
)

if(USE_GUI)
    target_compile_definitions(${MACHINE} PRIVATE -DUSE_GUI)
endif()

# 機種別の追加定義（実行ファイル用）
target_compile_definitions(${MACHINE} PRIVATE
    -D_RGB565
    -D_EXTEND_MENU
)

if(MACHINE STREQUAL "x1" OR MACHINE STREQUAL "x1turbo" OR MACHINE STREQUAL "x1turboz" OR MACHINE STREQUAL "x1twin")
    target_compile_definitions(${MACHINE} PRIVATE -D_HAS_TENKEY -DUSE_TV_CONTROL)
elseif(MACHINE STREQUAL "mz2200" OR MACHINE STREQUAL "mz2500" OR MACHINE STREQUAL "mz2800" OR
       MACHINE STREQUAL "mz3500" OR MACHINE STREQUAL "mz5500" OR MACHINE STREQUAL "mz6500" OR MACHINE STREQUAL "mz6550")
    target_compile_definitions(${MACHINE} PRIVATE -D_HAS_TENKEY)
elseif(MACHINE STREQUAL "msx1" OR MACHINE STREQUAL "msx2" OR MACHINE STREQUAL "msx2p")
    target_compile_definitions(${MACHINE} PRIVATE -D_MSX_VDP_MESS)
elseif(MACHINE STREQUAL "pc8801" OR MACHINE STREQUAL "pc8801ma" OR MACHINE STREQUAL "pc8801mk2")
    target_compile_definitions(${MACHINE} PRIVATE -D_HAS_TENKEY)
elseif(MACHINE STREQUAL "pc9801" OR MACHINE STREQUAL "pc9801e" OR MACHINE STREQUAL "pc9801ra" OR
       MACHINE STREQUAL "pc9801u" OR MACHINE STREQUAL "pc9801vf" OR MACHINE STREQUAL "pc9801vm" OR MACHINE STREQUAL "pc9801vx")
    target_compile_definitions(${MACHINE} PRIVATE -D_HAS_TENKEY)
elseif(MACHINE STREQUAL "bubcom80")
    target_compile_definitions(${MACHINE} PRIVATE -D_HAS_TENKEY)
elseif(MACHINE STREQUAL "pcengine")
    target_compile_definitions(${MACHINE} PRIVATE -D_HAS_TENKEY)
endif()

# C++ 側のライブラリとリンクする
target_link_libraries(${MACHINE} PRIVATE cscp)

# Objective-C++ 対応設定
set_target_properties(${MACHINE} PROPERTIES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
)

# Apple Frameworks
if(APPLE)
    find_library(FOUNDATION_LIB Foundation)
    find_library(METAL_LIB Metal)
    find_library(METALKIT_LIB MetalKit)
    find_library(QUARTZCORE_LIB QuartzCore)
    find_library(AUDIOTOOLBOX_LIB AudioToolbox)
    
    if(NOT IOS)
        find_library(APPKIT_LIB AppKit)
        target_link_libraries(${MACHINE} PRIVATE
            ${FOUNDATION_LIB}
            ${METAL_LIB}
            ${METALKIT_LIB}
            ${QUARTZCORE_LIB}
            ${APPKIT_LIB}
            ${AUDIOTOOLBOX_LIB}
        )
    else()
        find_library(UIKIT_LIB UIKit)
        target_link_libraries(${MACHINE} PRIVATE
            ${FOUNDATION_LIB}
            ${METAL_LIB}
            ${METALKIT_LIB}
            ${QUARTZCORE_LIB}
            ${UIKIT_LIB}
            ${AUDIOTOOLBOX_LIB}
        )
    endif()
endif()