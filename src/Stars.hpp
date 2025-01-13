//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

//----------------------------------------------------------

struct Star{
    int ID;
    float mag, ra, dec;
    std::string name;
};

class Stars{
private:
    sqlite3* db;

    std::vector<Star> stars;

public:
    Stars(const std::string& path);
    ~Stars();

    void fetchStarsbyMag(float maxmag, float minmag = -100);
    const std::vector<Star>& getStars() const { return stars; };

};