#pragma once
// ObjectDatabase.hpp --------------------------------------
// this file defines the class that is used for manipulating all of the data (ephemeris, pictures, object information etc)
#include <SFML/Window/Clipboard.hpp>

#include "pch.hpp"

#include "Ephemeris.hpp"
#include "Picture.hpp"
#include "net.hpp"

//----------------------------------------------------------



class ObjectDatabase{
private:
    // mean center is the offset coordinates of the middle point
    float m_mean_centerRa, m_mean_centerDec;
    // mean edge is the offset distance of points most distant from mean center
    float m_mean_edgeRa, m_mean_edgeDec;
    // center is the absolute coordinates of the 0, 0 ephemerid
    float m_centerRa, m_centerDec;
    int m_picExposure, m_picAmmount;
    // holds the percent of celected ephemeris inside existing pictures
    float selectedPercentage;
    // the name of the object
    std::string m_name;

    int telescope_FOV;

    std::string b10_to_b26(int c);

    void calculateSelected();

public:
    std::vector<Ephemeris> obj_data;
    std::vector<Picture> pictures;

    const std::tuple<float, float> mean_center() const;
    const std::tuple<float, float> mean_edges() const;
    const float selectedPercent() const;
    const std::string name() const;

    void set_FOV(int FOV);

    void set_exposure(int exp);
    void set_ammount(int amm);

    const int ephemeris_in_picture(float ra, float dec);

    void export_observation_targets(bool copy_cpb);

    void insert_data(std::string& str);

    void insert_picture(float ra, float dec);

    void remove_picture(int index);

    void undo_picture();

    void clear_pictures();

    const int closest_ephemeris_index(float ra, float dec);

    const int closest_picture_index(float ra, float dec);

    void reset();

    int fill_database(std::string lynk);

    ObjectDatabase();

};