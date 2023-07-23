//----------------------------------------------------------

#pragma once

#include <cpr/cpr.h>

#include "pch.hpp"

//----------------------------------------------------------

// The function that gets all of raw data from a provided url
// \param[in] link the url
// \param[out] raw the vector in which the data will be stored
// \param[in] milis max duration of the request (10000 ms by default)
// \throw DownloadFail, ForbiddenLink
void get_html(std::string& link, std::vector<std::string>& raw, int milis = 10000);
// due to the requirements of this program, raw is a vector where each string represents a single line
// that makes code somewhat more concise
