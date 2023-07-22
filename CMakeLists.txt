cmake_minimum_required(VERSION 3.24)

# project init
project(MPCS VERSION 3.0.0)

# set important vars
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(CMAKE_BUILD_FLAGS "-O3")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(WIN32) # Install dlls in the same directory as the executable on Windows
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# -------------------- DEPENDENCIES

include(FetchContent)

# add libcpr
FetchContent_Declare(cpr 
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.10.4 
    FIND_PACKAGE_ARGS NAMES cpr
)

FetchContent_GetProperties(cpr)
if(NOT cpr_POPULATED)
    FetchContent_Populate(cpr)

    set(CURL_ZLIB OFF CACHE INTERNAL "" FORCE)
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "" FORCE)

    add_subdirectory(${cpr_SOURCE_DIR} ${cpr_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# add glfw
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.3.8
    FIND_PACKAGE_ARGS NAMES glfw glfw3 libglfw libglfw3
)

FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)

    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
    set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
    set(ONLY_LIBS ON CACHE INTERNAL "" FORCE)

    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

# add glew-cmake
FetchContent_Declare(glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
    GIT_TAG 9758219375d923a008feef93c0955931d801740f # master branch Jun 26 2023
)

FetchContent_GetProperties(glew)
if(NOT glew_POPULATED)
    FetchContent_Populate(glew)

    set(glew-cmake_BUILD_SHARED OFF CACHE INTERNAL "" FORCE)

    add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR})
endif()

# add glm
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 0.9.9.8
    FIND_PACKAGE_ARGS NAMES glm libglm
)
FetchContent_MakeAvailable(glm)

# add inipp
FetchContent_Declare(inipp
    GIT_REPOSITORY https://github.com/mcmtroffaes/inipp
    GIT_TAG c61e699682d3f1091209c2179f1d03f5fc593327 # develop branch Feb 3 2022
)
FetchContent_MakeAvailable(inipp)

# add args
FetchContent_Declare(args
    GIT_REPOSITORY https://github.com/Taywee/args
    GIT_TAG 6.4.6
)

FetchContent_GetProperties(args)
if(NOT args_POPULATED)
    FetchContent_Populate(args)

    set(ARGS_BUILD_EXAMPLE OFF CACHE INTERNAL "" FORCE)
    set(ARGS_BUILD_UNITTESTS OFF CACHE INTERNAL "" FORCE)

    add_subdirectory(${args_SOURCE_DIR} ${args_BINARY_DIR})
endif()

# add libfmt
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 10.0.0
)
FetchContent_MakeAvailable(fmt)

# add clip
FetchContent_Declare(clip
    GIT_REPOSITORY https://github.com/dacap/clip
    GIT_TAG v1.5
)

FetchContent_GetProperties(clip)
if(NOT clip_POPULATED)
    FetchContent_Populate(clip)

    set(CLIP_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CLIP_TESTS OFF CACHE BOOL "" FORCE)

    add_subdirectory(${clip_SOURCE_DIR} ${clip_BINARY_DIR})
endif()

# -------------------- END OF DEPENDENCIES

# create executable
add_executable(${PROJECT_NAME})

# add code
add_subdirectory("src")

# include submodules
target_include_directories(${PROJECT_NAME} PUBLIC
    "${PROJECT_BINARY_DIR}/_deps/args-src/" # header only
    "${PROJECT_BINARY_DIR}/_deps/inipp-src/" # header only
    "${PROJECT_BINARY_DIR}/_deps/glm-src/"
    "${PROJECT_BINARY_DIR}/_deps/clip-src/"
    "${PROJECT_BINARY_DIR}/_deps/fmt-src/include/"
    "${PROJECT_BINARY_DIR}/_deps/glew-src/include/"
    "${PROJECT_BINARY_DIR}/_deps/glfw-src/include/"
    "${PROJECT_BINARY_DIR}/_deps/cpr-src/include/"
)

# link all libraries to the executable
target_link_libraries(${PROJECT_NAME}
    "cpr;glfw;libglew_static;fmt;glm;clip"
)

#copy the resources to the build folder
if(NOT EXISTS "./build/resources")
    file(COPY "./resources" DESTINATION ".")
    message("> Resources copied")
endif()

#install rules
#binary
install(
    TARGETS ${PROJECT_NAME}
    DESTINATION "bin"
)
#resources
install(
    DIRECTORY "resources"
    DESTINATION "."
)
install(
    FILES "LICENSE.txt"
    DESTINATION "."
)

# for windows systems, copy dlls to the bin folder
if (WIN32)
    if (MINGW)
        get_filename_component(Mingw_Path ${CMAKE_CXX_COMPILER} PATH)
        file(GLOB RUNTIME_LIBS 
            "${Mingw_Path}/libcrypto*.dll"
            "${Mingw_Path}/libwinpthread*.dll"
            "${Mingw_Path}/libssh*.dll"
            "${Mingw_Path}/zlib*.dll"
        )
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${RUNTIME_LIBS})
    endif()

    if (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT System)
    else()
        message("No required runtime libs found!")
    endif()
endif()

#CPack installer setup
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${MPCS_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MPCS_VERSION_MINOR}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/astrohr/MPCS")
include(CPack)
