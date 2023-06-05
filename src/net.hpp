#pragma once
// net.hpp -------------------------------------------------
// this file controls the curl requsts that the program executes

#include <curl/curl.h>
#include <SFML/System/Clock.hpp>

#include "pch.hpp"

//----------------------------------------------------------



extern std::vector<std::string> g_allowed_links; //this gets populated in main.cpp::default_variables()

static size_t progress_callback(void* approx_size, double dltotal, double dlnow, double ultotal, double ulnow);

size_t read_curl_data(char* ptr, size_t size, size_t nmemb, std::vector<std::string>* userdata);

int get_html(std::string link, std::vector<std::string>* userdata, int milis = 10000);

