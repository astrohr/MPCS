cmake_minimum_required(VERSION 3.20)

project(MPCS VERSION 2.6.0)

if(NOT EXISTS "./build")
    file(MAKE_DIRECTORY "build")
    message("> build folder created")
endif()

set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)

set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
if (WIN32)
    set(SFML_USE_STATIC_STD_LIBS ON)
endif()

set(BUILD_CURL_EXE OFF) 
set(CURL_STATICLIB ON)
set(HTTP_ONLY ON)
set(CURL_ENABLE_SSL ON)
if (WIN32)
    add_definitions(-DCURL_STATICLIB)
    set(CURL_USE_SCHANNEL ON)
endif()

set(CMAKE_BUILD_FLAGS "-O3")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# set path variables so its easier to read
set(SUBMODULES "${PROJECT_SOURCE_DIR}/submodules")
set(SRC "${PROJECT_SOURCE_DIR}/src")

add_subdirectory("${SUBMODULES}/SFML")
add_subdirectory("${SUBMODULES}/curl")
add_subdirectory("${SUBMODULES}/fmt")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

configure_file("${SRC}/cmakevars.h.in" "${PROJECT_BINARY_DIR}/src/cmakevars.h")

add_executable(${PROJECT_NAME}
    "${SRC}/main.cpp"
    "${SRC}/Camera.cpp"
    "${SRC}/net.cpp"
    "${SRC}/ObjectDatabase.cpp"
    "${SRC}/Ephemeris.cpp"
    "${SRC}/Picture.cpp"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SRC}/pch.hpp")

target_include_directories(${PROJECT_NAME} PUBLIC
    "${SUBMODULES}/SFML/include"
    "${SUBMODULES}/curl/include"
    "${SUBMODULES}/fmt/include"
    "${SUBMODULES}/args"
    "${SUBMODULES}/inipp"
    "${PROJECT_BINARY_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    "libcurl;sfml-system;sfml-window;sfml-graphics;fmt"
)

#copy the resources
if(NOT EXISTS "./build/resources")
    file(COPY "./resources" DESTINATION ".")
    message("> Resources copied")
endif()

#copy MPCS.ini
if(NOT EXISTS "./build/MPCS.ini")
    file(COPY "./MPCS.ini" DESTINATION ".")
    message("> MPCS.ini copied")
endif()


unset(SUBMODULES)
unset(SRC)
