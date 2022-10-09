configure_file(cmakevars.h.in cmakevars.h)

add_executable(${PROJECT_NAME} main.cpp Camera.cpp net.cpp ObjectDatabase.cpp Ephemeris.cpp Picture.cpp)