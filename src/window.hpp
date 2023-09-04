//----------------------------------------------------------

#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "pch.hpp"
#include "utils/utils.hpp"

#include "graphics/GLProgram.hpp"
#include "graphics/Camera.hpp"

#include "Object.hpp"
#include "Observatory.hpp"

//----------------------------------------------------------


void windowFunction(unsigned int W, unsigned int H, std::vector<Object>& objects, Observatory& observatory);
