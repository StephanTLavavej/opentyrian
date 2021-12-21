# .\vcpkg\bootstrap-vcpkg.bat
# .\vcpkg\vcpkg.exe install sdl2:x64-windows sdl2-net:x64-windows
# "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvars64.bat"
# cmake -Wno-dev -DCMAKE_BUILD_TYPE=Debug   -G Ninja -S . -B out\dbg && ninja -C out\dbg
# cmake -Wno-dev -DCMAKE_BUILD_TYPE=Release -G Ninja -S . -B out\rel && ninja -C out\rel

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

cmake_minimum_required(VERSION 3.22)

project(opentyrian LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/animlib.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/arg_parse.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/backgrnd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/config_file.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/destruct.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/editship.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/episodes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/file.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/font.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/fonthand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/game_menu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/helptext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/joystick.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/jukebox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/keyboard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/lds_play.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/loudness.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/lvllib.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/lvlmast.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mainint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/menus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mouse.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mtrand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/musmast.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/network.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/nortsong.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/nortvars.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/opentyr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/opl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/palette.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/params.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/pcxload.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/pcxmast.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/picload.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/player.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/shots.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sizebuf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sndmast.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sprite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/starlib.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/tyrian2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/varz.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/vga_palette.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/vga256d.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video_scale.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/video_scale_hqNx.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/xmas.cpp
)

add_executable(opentyrian ${SOURCES})

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(opentyrian PRIVATE SDL2::SDL2 SDL2::SDL2main)

find_package(sdl2-net CONFIG REQUIRED)
target_link_libraries(opentyrian PRIVATE SDL2::SDL2_net)
target_compile_definitions(opentyrian PRIVATE WITH_NETWORK)

target_compile_definitions(opentyrian PRIVATE TYRIAN_DIR="tyrian21")

if (WIN32)
    target_compile_definitions(opentyrian PRIVATE TARGET_WIN32)
endif()

if (MSVC)
    target_sources(opentyrian PRIVATE ${CMAKE_CURRENT_LIST_DIR}/visualc/resources.rc)

    # https://discourse.cmake.org/t/noisy-output-from-rc-compiler/3463
    set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} /nologo")

    target_compile_definitions(opentyrian PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(opentyrian PRIVATE /W4 /WX /sdl /analyze)

    target_link_options(opentyrian PRIVATE $<$<CONFIG:Debug>:/OPT:REF,NOICF>)

    target_compile_options(opentyrian PRIVATE $<$<CONFIG:Release>:/GL>)
    target_link_options(opentyrian PRIVATE $<$<CONFIG:Release>:/LTCG /OPT:REF,ICF>)

    # 4 occurrences of warning C4018: signed/unsigned mismatch
    target_compile_options(opentyrian PRIVATE /wd4018)

    # 525 occurrences of warning C4244: conversion from X to Y, possible loss of data
    target_compile_options(opentyrian PRIVATE /wd4244)

    # 5 occurrences of warning C4245: conversion from X to Y, signed/unsigned mismatch
    target_compile_options(opentyrian PRIVATE /wd4245)

    # 14 occurrences of warning C4267: conversion from X to Y, possible loss of data
    target_compile_options(opentyrian PRIVATE /wd4267)

    # 5 occurrences of warning C4305: truncation from X to Y
    target_compile_options(opentyrian PRIVATE /wd4305)

    # 14 occurrences of warning C4456: declaration of X hides previous local declaration
    target_compile_options(opentyrian PRIVATE /wd4456)

    # 4 occurrences of warning C4457: declaration of X hides function parameter
    target_compile_options(opentyrian PRIVATE /wd4457)

    # 220 occurrences of warning C4459: declaration of X hides global declaration
    target_compile_options(opentyrian PRIVATE /wd4459)

    # 1 occurrence of warning C4702: unreachable code
    target_compile_options(opentyrian PRIVATE /wd4702)

    ##### /analyze warnings: #####

    # 1 occurrence of warning C6001: Using uninitialized memory X
    target_compile_options(opentyrian PRIVATE /wd6001)

    # 11 occurrences of warning C6011: Dereferencing NULL pointer X
    target_compile_options(opentyrian PRIVATE /wd6011)

    # 2 occurrences of warning C6031: Return value ignored: '_mkdir'
    target_compile_options(opentyrian PRIVATE /wd6031)

    # 175 occurrences of warning C6244: Local declaration of X hides previous declaration at LINE of FILE
    target_compile_options(opentyrian PRIVATE /wd6244)

    # 9 occurrences of warning C6246: Local declaration of X hides declaration of the same name in outer scope
    target_compile_options(opentyrian PRIVATE /wd6246)

    # 1 occurrence of warning C6262: Function uses '78416' bytes of stack: exceeds /analyze:stacksize '16384'. Consider moving some data to heap.
    target_compile_options(opentyrian PRIVATE /wd6262)

    # 1 occurrence of warning C6297: Arithmetic overflow: 32-bit value is shifted, then cast to 64-bit value. Results might not be an expected value.
    target_compile_options(opentyrian PRIVATE /wd6297)

    # 29 occurrences of warning C6340: Mismatch on sign: X passed as _Param_(N) when some signed type is required in call to Y
    # Need to report an issue to Visual Studio Developer Community: This warning doesn't consider integer promotions.
    target_compile_options(opentyrian PRIVATE /wd6340)

    # 20 occurrences of warning C6385: Reading invalid data from X: the readable size is Y bytes, but Z bytes may be read
    target_compile_options(opentyrian PRIVATE /wd6385)

    # 3 occurrences of warning C6386: Buffer overrun while writing to X: the writable size is Y bytes, but Z bytes might be written
    target_compile_options(opentyrian PRIVATE /wd6386)

    # 6 occurrences of warning C6387: X could be '0': this does not adhere to the specification for the function Y
    target_compile_options(opentyrian PRIVATE /wd6387)

    # Many occurrences (only in C) of warning C28251: Inconsistent annotation for '_setjmp': this instance has no annotations. See <no file>(0).
    target_compile_options(opentyrian PRIVATE /wd28251)
endif()
