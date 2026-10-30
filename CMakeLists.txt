# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/components/boards"
    "${CMAKE_CURRENT_LIST_DIR}/components/basic"
    "${CMAKE_CURRENT_LIST_DIR}/components/communication"
    "${CMAKE_CURRENT_LIST_DIR}/components/display"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP32_V6_myref)
