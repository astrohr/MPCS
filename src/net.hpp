#pragma once

#include <curl/curl.h>
#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <map>

extern std::vector<std::string> g_allowed_links; //this gets populated in default_variables()

static size_t progress_callback(void* approx_size, double dltotal, double dlnow, double ultotal, double ulnow);

size_t read_curl_data(char* ptr, size_t size, size_t nmemb, std::vector<std::string>* userdata);

int get_html(std::string link, std::vector<std::string>* userdata, double size = 0.0, int milis = 20000);

