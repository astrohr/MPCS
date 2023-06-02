cmake_minimum_required(VERSION 3.20)

project(MPCS VERSION 2.6.0)

if(NOT EXISTS "./build")
    file(MAKE_DIRECTORY "build")
    message("> build folder created")
endif()

set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_CURL_EXE OFF) 
set(BUILD_TESTING OFF)
set(CMAKE_BUILD_FLAGS "-O3")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CURL_ENABLE_SSL ON)
if (WIN32)
    set(SFML_USE_STATIC_STD_LIBS ON)
    set(CURL_USE_SCHANNEL ON)
endif()

add_subdirectory(external/SFML/)
add_subdirectory(external/curl/)
add_subdirectory(external/fmt/)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

configure_file(src/cmakevars.h.in src/cmakevars.h)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/Camera.cpp
    src/net.cpp
    src/ObjectDatabase.cpp
    src/Ephemeris.cpp
    src/Picture.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.hpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC external/SFML/include
    PUBLIC external/curl/include
    PUBLIC external/fmt/include
    PUBLIC external/args
    PUBLIC "${PROJECT_BINARY_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    "sfml-system;sfml-window;sfml-graphics;libcurl;fmt"
)

#copy the resources
if(NOT EXISTS "./build/resources")
    file(COPY "./resources" DESTINATION ".")
    message("> Resources copied")
endif()

#copy data
if(NOT EXISTS "./build/data")
    file(COPY "./data" DESTINATION ".")
    message("> Data copied")
endif()
