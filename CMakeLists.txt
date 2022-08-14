cmake_minimum_required(VERSION 3.24)

project(MPCS VERSION 2.0)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(external/SFML/)

target_include_directories(${PROJECT_NAME}
    PUBLIC external/SFML/include
)

target_link_directories(${PROJECT_NAME}
    PRIVATE external/SFML/lib
)

target_link_libraries(${PROJECT_NAME}
    "sfml-system;sfml-window;sfml-graphics"
)

#copy the resources
if(NOT EXISTS "./build/resources")
    file(COPY "./resources" DESTINATION ".")
endif()

#copy data
if(NOT EXISTS "./build/data")
    file(COPY "./data" DESTINATION ".")
endif()