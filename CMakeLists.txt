cmake_minimum_required(VERSION 3.20)

# project init
project(MPCS VERSION 2.6.0)

# make build folder
if(NOT EXISTS "./build")
    file(MAKE_DIRECTORY "build")
    message("> build folder created")
endif()

# set important vars
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(CMAKE_BUILD_FLAGS "-O3")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# set sfml vars
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
if (WIN32)
    set(SFML_USE_STATIC_STD_LIBS ON)
endif()

# set curl vars
set(BUILD_CURL_EXE OFF) 
set(CURL_STATICLIB ON)
set(HTTP_ONLY ON)
set(CURL_ENABLE_SSL ON)
if (WIN32)
    add_definitions(-DCURL_STATICLIB)
    set(CURL_USE_SCHANNEL ON)
endif()

# add submodules
add_subdirectory("submodules/SFML")
add_subdirectory("submodules/curl")
add_subdirectory("submodules/fmt")

# create executable
add_executable(${PROJECT_NAME})

# add code
add_subdirectory("src")

# include submodules
target_include_directories(${PROJECT_NAME} PUBLIC
    "submodules/SFML/include"
    "submodules/curl/include"
    "submodules/fmt/include"
    "submodules/args"
    "submodules/inipp"
)

# link all libraries to the executable
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
