cmake_minimum_required(VERSION 3.4)
project(zapPlayer)

set(Qt4and5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(APPLE)
    set(CMAKE_CXX_STANDARD 14)
    add_definitions(-Wall -Werror)
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} - std=c++1y")
    add_definitions(-Wall -Werror)
elseif(WIN32)

endif(APPLE)

find_package(zap REQUIRED PATHS ${CMAKE_SOURCE_DIR}/third_party NO_DEFAULT_PATH)
find_package(zapAudio REQUIRED PATHS ${CMAKE_SOURCE_DIR}/third_party NO_DEFAULT_PATH)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)

qt5_wrap_cpp(ZAP_PLAYER_MOC
        QZapWidget.h
        zapPlayer.h)

qt5_wrap_ui(ZAP_PLAYER_UI zapPlayer.ui)

set(ZAP_PLAYER_FILES main.cpp zapPlayer.cpp QZapWidget.cpp visualiser.cpp visualiser.hpp analyser_stream.cpp analyser_stream.hpp directory_stream.cpp directory_stream.hpp controller_stream.hpp module/module.hpp analyser.cpp analyser.hpp module/histogram.cpp module/histogram.hpp module/spectrogram.cpp module/spectrogram.hpp)

add_executable(zapPlayer ${ZAP_PLAYER_FILES} ${ZAP_PLAYER_MOC} ${ZAP_PLAYER_UI})
target_include_directories(zapPlayer PUBLIC ${zap_INCLUDE_DIRS} ${zapAudio_INCLUDE_DIRS})

if(APPLE)
    target_link_libraries(zapPlayer ${zap_LIBRARIES} ${zapAudio_LIBRARIES} Qt5::Widgets Qt5::Gui Qt5::OpenGL)
elseif(WIN32)
    target_link_libraries(zapPlayer ${zap_LIBRARIES} ${zapAudio_LIBRARIES} Qt5::Widgets Qt5::Gui Qt5::OpenGL)
endif(APPLE)
