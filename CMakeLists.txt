cmake_minimum_required(VERSION 3.24)

# project init
project(MPCS VERSION 3.0.0)

# set important vars
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(CMAKE_BUILD_FLAGS "-O3")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(WIN32) # Install dlls in the same directory as the executable on Windows
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# set sfml vars
set(SFML_BUILD_AUDIO OFF)
set(SFML_BUILD_NETWORK OFF)
if(WIN32)
  set(SFML_USE_STATIC_STD_LIBS ON)
endif()

include(FetchContent)

# add libcpr
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.4
  FIND_PACKAGE_ARGS NAMES cpr)

# since no large data is being sent, zlib isnt needed this also doesnt work all
# that well, so yeah, definetely a TODO
set(CURL_ZLIB OFF)

if(NOT cpr_POPULATED)
  FetchContent_Populate(cpr)
  add_subdirectory(${cpr_SOURCE_DIR} ${cpr_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# add inipp
FetchContent_Declare(
  inipp
  GIT_REPOSITORY https://github.com/mcmtroffaes/inipp
  GIT_TAG c61e699682d3f1091209c2179f1d03f5fc593327 # develop branch Feb 3 2022
)
FetchContent_MakeAvailable(inipp)

# add args
FetchContent_Declare(
  args
  GIT_REPOSITORY https://github.com/Taywee/args
  GIT_TAG 6.4.6)
FetchContent_MakeAvailable(args)

# add libfmt
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 10.0.0)
FetchContent_MakeAvailable(fmt)

# add submodules
add_subdirectory("submodules/SFML")

# create executable
add_executable(${PROJECT_NAME})

# add code
add_subdirectory("src")

# include submodules
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC # cmake expects the dependencies to have an include folder for headers,
         # these projects dont so i place include path manually
         "${PROJECT_BINARY_DIR}/_deps/args-src/"
         "${PROJECT_BINARY_DIR}/_deps/inipp-src/" "submodules/SFML/include")

# link all libraries to the executable
target_link_libraries(${PROJECT_NAME}
                      "cpr::cpr;sfml-system;sfml-window;sfml-graphics;fmt::fmt")

# copy the resources to the build folder
if(NOT EXISTS "./build/resources")
  file(COPY "./resources" DESTINATION ".")
  message("> Resources copied")
endif()

# install rules binary
install(TARGETS ${PROJECT_NAME} DESTINATION "bin")
# resources
install(DIRECTORY "resources" DESTINATION ".")
install(FILES "LICENSE.txt" DESTINATION ".")

# for windows systems, copy dlls to the bin folder
if(WIN32)
  if(MINGW)
    get_filename_component(Mingw_Path ${CMAKE_CXX_COMPILER} PATH)
    file(GLOB RUNTIME_LIBS "${Mingw_Path}/libcrypto*.dll"
         "${Mingw_Path}/libwinpthread*.dll" "${Mingw_Path}/libssh*.dll"
         "${Mingw_Path}/zlib*.dll")
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${RUNTIME_LIBS})
  endif()

  if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    install(
      PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
      DESTINATION bin
      COMPONENT System)
  else()
    message("No required runtime libs found!")
  endif()
endif()

# CPack installer setup
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${MPCS_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MPCS_VERSION_MINOR}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/astrohr/MPCS")
include(CPack)
