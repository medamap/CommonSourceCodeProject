cmake_minimum_required(VERSION 3.15)
project(CSCP_iPadOS LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

# Appleプラットフォーム前提
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

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
    -DUSE_GUI
)

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
)

if(USE_GUI)
    target_compile_definitions(${MACHINE} PRIVATE -DUSE_GUI)
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