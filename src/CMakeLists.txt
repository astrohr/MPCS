configure_file(cmakevars.h.in cmakevars.h)

add_executable(${PROJECT_NAME} 
    main.cpp
    Camera.cpp
    net.cpp
    ObjectDatabase.cpp
    Ephemeris.cpp
    Picture.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.hpp)