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

class ConstructorFail : public std::runtime_error
{
public:
    ConstructorFail(const std::string& problem) : std::runtime_error(problem) {}
    virtual ~ConstructorFail() {}
};

}