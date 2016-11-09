cmake_minimum_required(VERSION 3.6)
project(zapPlayer)

set(Qt4and5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(APPLE)
    set(CMAKE_CXX_STANDARD 14)
    add_definitions(-Wall -Werror -DGLEW_STATIC)
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} - std=c++1y")
    add_definitions(-Wall -Werror)
elseif(WIN32)

endif(APPLE)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)

qt5_wrap_cpp(ZAP_PLAYER_MOC zapPlayer.h)

qt5_wrap_ui(ZAP_PLAYER_UI zapPlayer.ui)

set(ZAP_PLAYER_FILES main.cpp zapPlayer.cpp)

add_executable(zapPlayer ${ZAP_PLAYER_FILES} ${ZAP_PLAYER_MOC} ${ZAP_PLAYER_UI})

if(APPLE)
    target_link_libraries(zapPlayer Qt5::Widgets Qt5::Gui Qt5::OpenGL)
elseif(WIN32)
    target_link_libraries(zapPlayer Qt5::Widgets Qt5::Gui Qt5::OpenGL)
endif(APPLE)
