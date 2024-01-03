//----------------------------------------------------------

#pragma once

//----------------------------------------------------------

// streams
#include <iostream>
#include <fstream>
#include <sstream>

// functionalities
#include <filesystem>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <ctime>
#include <regex>

// math
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <numbers>
#include <cmath>

// data structures
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include <tuple>
#include <array>
#include <list>
#include <map>

// external libs
#include <fmt/core.h>

// custom
#include "utils/Log.hpp"
extern Log logs;
extern std::map<std::string, int> monthVals;
extern std::string g_resourcesPath; // path to MPCS.ini
