//----------------------------------------------------------

#pragma once

#include "Graphics.hpp"

#include "pch.hpp"
#include "utils/utils.hpp"

#include "graphics/GLProgram.hpp"
#include "graphics/Camera.hpp"

#include "Object.hpp"
#include "Stars.hpp"
#include "PointCluster.hpp"
#include "Circle.hpp"
#include "Grid.hpp"
#include "Observatory.hpp"

//----------------------------------------------------------


void windowFunction(unsigned int W, unsigned int H, std::vector<Object>& objects, Stars& stars, Observatory& observatory);
