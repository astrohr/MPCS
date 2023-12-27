//----------------------------------------------------------

#include "AutoPic.hpp"

//----------------------------------------------------------

std::vector<std::pair<int, int>> AutoPickPictures(ObjectDatabase& database, unsigned int fov, int step_size /*= 10*/, float min_coverage /*= 0.95*/, float density /*= 0.04*/)
{
    float captured_ephems_number = 0; // count total number of captured ephemerids
    const int total_ephems = database.getEphs().size(); // for debug: stores total number of ephemerids
    std::vector<int> fov_captures = {}; // for debug: stores how many points were captured per picture
    std::vector<std::pair<int, int>> removeable_ephemerids = {}; // stores ephemerids not captured yet
    // fill removeable_ephemerids
    for (const auto& eph : database.getEphs()) {
        auto [x, y] = eph.getOffsets();
        removeable_ephemerids.emplace_back(std::make_pair(x, y));
    }
    std::vector<std::pair<int, int>> picked_ephemerids = {}; // stores the coordinates of the pictures
    std::vector<std::pair<std::pair<int, int>, std::vector<std::pair<int, int>>>> sorted_ephem_fovs; // stores picture center and ephemerides it captured

    do {
        sorted_ephem_fovs.clear(); // clear the vector of potential pictures every iteration

        for (size_t i = 0; i < database.getEphs().size(); i += step_size) { // iterate over potential picture centers
            auto eph = database.getEphs()[i];
            auto [x, y] = eph.getOffsets(); // picture center candidate
            std::vector<std::pair<int, int>> captured_ephems_temp = {};
            for (const auto& eph_temp : removeable_ephemerids) { // go through all ephemerids not captured yet
                auto [x_temp, y_temp] = eph_temp;

                if (abs(x_temp - x) < fov / 2.f && abs(y_temp - y) < fov / 2.f) { // check if ephemerid is in fov of candidate picture
                    captured_ephems_temp.emplace_back(std::make_pair(x_temp, y_temp));
                }
            }
            sorted_ephem_fovs.emplace_back(std::make_pair(std::make_pair(x, y), captured_ephems_temp));
        }

        // sort ephem_fovs by number of captured ephemerids, greatest to lowest
        std::sort(sorted_ephem_fovs.begin(), sorted_ephem_fovs.end(), [](const auto& a, const auto& b) {
            return a.second.size() > b.second.size();
        });

        captured_ephems_number += sorted_ephem_fovs[0].second.size();
        picked_ephemerids.emplace_back(sorted_ephem_fovs[0].first);
        fov_captures.emplace_back(sorted_ephem_fovs[0].second.size());

        auto to_remove = sorted_ephem_fovs[0].second; // vector of ephemerids to remove from removeable_ephemerids
        removeable_ephemerids.erase(std::remove_if(removeable_ephemerids.begin(), removeable_ephemerids.end(),
                                        [&to_remove](const auto& el) {
                                            return std::find(to_remove.begin(), to_remove.end(), el) != to_remove.end();
                                        }),
            removeable_ephemerids.end());

        std::cout << "Enough points? " << (captured_ephems_number / total_ephems < min_coverage) << captured_ephems_number / total_ephems << ", " << min_coverage << std::endl;
        std::cout << "Enough dense? " << (sorted_ephem_fovs[0].second.size() > density * captured_ephems_number) << std::endl;
    } while (captured_ephems_number / total_ephems < min_coverage && sorted_ephem_fovs[0].second.size() > density * captured_ephems_number);

    // debugging
    std::cout << "Number of points: " << captured_ephems_number;
    std::cout << " Number of fovs: " << picked_ephemerids.size() << std::endl;
    for (auto point : picked_ephemerids) {
        std::cout << "(" << point.first << ", " << point.second << ")  ";
    }
    std::cout << std::endl;
    for (auto point : fov_captures) {
        std::cout << point << "  ";
    }
    std::cout << std::endl;

    return picked_ephemerids;
}

void InsertAutoPickedPictures(ObjectDatabase& database, const std::vector<std::pair<int, int>>& picked_ephemerids)
{
    for (const auto& point : picked_ephemerids) {
        float x = static_cast<float>(point.first);
        float y = static_cast<float>(point.second);
        database.insert_picture(x, y);
    }
}