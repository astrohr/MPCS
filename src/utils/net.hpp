//----------------------------------------------------------

#pragma once

#include <cpr/cpr.h>

#include "pch.hpp"

//----------------------------------------------------------


// The function that gets all of raw data from a provided url
// \param[in] link the url
// \param[in] milis max duration of the request (10000 ms by default)
// \returns raw data string of the response
// \throw DownloadFail, ForbiddenLink
std::string get_html(std::string& link, int milis = 10000);

// The function that gets all of raw data from a provided url
// \param[in] link the url
// \param[out] raw the vector in which the data will be stored, each string is a separate line
// \param[in] milis max duration of the request (10000 ms by default)
// \throw DownloadFail, ForbiddenLink
void get_html(std::string& link, std::vector<std::string>& raw, int milis = 10000);
