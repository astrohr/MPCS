//----------------------------------------------------------

#pragma once

#include "pch.hpp"

//----------------------------------------------------------

// custom exceptions/errors
namespace mpcsError
{

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

class BadData : public std::runtime_error
{
public:
    BadData(const std::string& problem) : std::runtime_error(problem) {}
    virtual ~BadData() {}
};

class InippError : public std::runtime_error
{
public:
    InippError(const std::string& problem) : std::runtime_error(problem) {}
    virtual ~InippError() {}
};

}