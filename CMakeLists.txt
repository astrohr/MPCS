cmake_minimum_required(VERSION 3.20)

project(MPCS VERSION 2.0)

add_executable(${PROJECT_NAME} main.cpp)

set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_CURL_EXE OFF) 
set(BUILD_TESTING OFF)

set(CURL_ENABLE_SSL ON)
if (WIN32)
    set(SFML_USE_STATIC_STD_LIBS ON)
    set(CURL_USE_SCHANNEL ON)
endif()

add_subdirectory(external/SFML/)
add_subdirectory(external/curl/)

target_include_directories(${PROJECT_NAME}
    PUBLIC external/SFML/include
    PUBLIC external/curl/include
)

target_link_libraries(${PROJECT_NAME}
    "sfml-system;sfml-window;sfml-graphics;libcurl"
)

#copy the resources
if(NOT EXISTS "./build/resources")
    file(COPY "./resources" DESTINATION ".")
endif()

#copy data
if(NOT EXISTS "./build/data")
    file(COPY "./data" DESTINATION ".")
endif()