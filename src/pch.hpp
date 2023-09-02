//----------------------------------------------------------

#pragma once

//----------------------------------------------------------

// streams
#include <iostream>
#include <fstream>
#include <sstream>

// functionalities
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <format>
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
extern std::string g_mpcsIniPath; // path to MPCS.ini
extern const double g_radian; // how many radians in a degree
extern const time_t g_siderealDayLength; // how many miliseconds in a sidereal day
extern std::pair<time_t, time_t> g_siderealTimeReference; // a reference for sidereal time, first is 00:00 at Greenwich, second is its sidereal time
