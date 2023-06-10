//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include <curl/curl.h>
#include <SFML/System/Clock.hpp>

//----------------------------------------------------------


// the links that the program is allowed to go to esentially
extern std::vector<std::string> g_allowed_links;

// https://curl.se/libcurl/c/CURLOPT_PROGRESSFUNCTION.html
static size_t progress_callback(void* approx_size, double dltotal, double dlnow, double ultotal, double ulnow);

// functions that reads data from a url location
// https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
size_t read_curl_data(char* ptr, size_t size, size_t nmemb, std::vector<std::string>* userdata);

// The function that gets all of raw data from a provided url
// \param[in] link the url
// \param[out] userdata the vector in which the data will be stored
// \param[in] milis max duration of the request (10000 ms if not provided)
// \return 0/1 if execution failed or not
bool get_html(std::string& link, std::vector<std::string>& userdata, int milis = 10000);
// due to the requirements of this program, userdata is a vector where each string represents a single line
// that makes code somewhat more concise
