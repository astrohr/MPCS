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

# for mingw, copy dlls
if (MINGW)
    get_filename_component(Mingw_Path ${CMAKE_CXX_COMPILER} PATH)

    # i swear, the existence of this command annoys me so much, cant WAIT to figure a way to REMOVE it
    # if someone starts defending libcurl just show them dis
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS 
        ${Mingw_Path}/libwinpthread-1.dll 
        ${Mingw_Path}/libssh2-1.dll
        ${Mingw_Path}/libidn2-0.dll 
        ${Mingw_Path}/libpsl-5.dll
        ${Mingw_Path}/zlib1.dll
        ${Mingw_Path}/libiconv-2.dll
        ${Mingw_Path}/libintl-8.dll
        ${Mingw_Path}/libunistring-2.dll
        ${Mingw_Path}/libcrypto-1_1-x64.dll
    )
endif()

if (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT System)
endif()

#CPack installer setup
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${MPCS_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MPCS_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MPCS_VERSION_PATCH}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/astrohr/MPCS")
if (WIN32)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_SOURCE_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
    set(CPACK_SOURCE_GENERATOR TGZ)
endif()
include(CPack)
