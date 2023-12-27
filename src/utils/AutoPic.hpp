//----------------------------------------------------------

#pragma once

#include "./ObjectDatabase.hpp"
//#include "pch.hpp"
//----------------------------------------------------------

// Uses an algorithm do determine optimal locations of Pictures
// \param database the database instance to use
// \param fov the field of view of the pictures
// \param step_size the step size of the algorithm
// \param min_coverage desired minimum coverage of the ephemerids
// \param density desired density of the ephemerids per picture
// \returns a vector of pairs of pictures coordinates
std::vector<std::pair<int, int>> AutoPickPictures(ObjectDatabase& database, unsigned int fov, int step_size = 10, float min_coverage = 0.95, float density = 0.04);

// Inserts the pictures into the database
// \param database the database instance to use
// \param picked_ephemerids the vector of pairs of pictures coordinates
void InsertAutoPickedPictures(ObjectDatabase& database, const std::vector<std::pair<int, int>>& picked_ephemerids);
