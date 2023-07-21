//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include <cpr/cpr.h>

//----------------------------------------------------------

namespace utils
{


// net.cpp

// The function that gets all of raw data from a provided url
// \param[in] link the url
// \param[out] raw the vector in which the data will be stored
// \param[in] milis max duration of the request (10000 ms by default)
// \throw DownloadFail, ForbiddenLink
void get_html(std::string& link, std::vector<std::string>& raw, int milis = 10000);
// due to the requirements of this program, raw is a vector where each string represents a single line
// that makes code somewhat more concise


// custom exceptions/errors

class DownloadFail : public std::runtime_error
{
public:
    DownloadFail(const std::string& problem) : std::runtime_error(problem) {}
    virtual ~DownloadFail() {}
};

class ForbiddenLink : public std::runtime_error
{
public:
    ForbiddenLink(const std::string& problem) : std::runtime_error(problem) {}
    virtual ~ForbiddenLink() {}
};

class ConstructorFail : public std::runtime_error
{
public:
    ConstructorFail(const std::string& problem) : std::runtime_error(problem) {}
    virtual ~ConstructorFail() {}
};

class GraphicsError : public std::runtime_error
{
public:
    GraphicsError(const std::string& problem) : std::runtime_error(problem) {}
    virtual ~GraphicsError() {}
};

}