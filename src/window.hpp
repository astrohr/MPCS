//----------------------------------------------------------

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "pch.hpp"
#include "utils/utils.hpp"
#include "Camera.hpp"

//----------------------------------------------------------

//void WindowSetup(ObjectDatabase& database, Camera& cam);

// \throws GraphicsError
void windowFunction(ObjectDatabase& database, Camera& cam);
