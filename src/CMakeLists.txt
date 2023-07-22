# add sources and a precompiled header
target_sources(${PROJECT_NAME} PUBLIC
    "main.cpp"
    "Camera.cpp"
    "ObjectDatabase.cpp"
    "Ephemeris.cpp"
    "Picture.cpp"
    "utils/net.cpp"
    "utils/Log.cpp"
)
target_precompile_headers(${PROJECT_NAME} PRIVATE "pch.hpp")

# write defines to a file
configure_file("cmakevars.h.in" "${PROJECT_BINARY_DIR}/src/cmakevars.h")

# include all directories that this code requires (except for the external dependencies)
target_include_directories(${PROJECT_NAME} PUBLIC
    "${PROJECT_BINARY_DIR}" # due to the configured file
    "utils"
    "."
)