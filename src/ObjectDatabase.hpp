//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include <SFML/Window/Clipboard.hpp>

#include "Ephemeris.hpp"
#include "Picture.hpp"
#include "net.hpp"

//----------------------------------------------------------



class ObjectDatabase{
private:

    // the exposure of the pictures
    int m_picExposure;
    // the amount of the pictures
    int m_picAmount;

    // Absolute coordinates of the 0,0 point
    float m_centerRa, m_centerDec;
    
    // object name
    std::string m_name;

    // this one is quite self explainatory, isnt it?
    unsigned int m_telescope_FOV;

    // function for converting numbers to characters (in a base 26 way)
    std::string b10_to_b26(int c);

    // ephemerides storage
    std::vector<Ephemeris> m_ephemerides;
    
    // picture area storage
    std::vector<Picture> m_pictures;

public:

    ObjectDatabase() = default;

    
    
    // Object name getter
    const std::string name() const
    { return m_name; }

    // FOV setter
    void set_FOV(unsigned int& FOV)
    { m_telescope_FOV = FOV; }

    // FOV getter
    const unsigned int getFOV() const
    { return m_telescope_FOV; }

    // Sets the exposure of pictures to be taken
    void set_exposure(int& exp)
    { m_picExposure = exp; }
    
    // Sets the amount of pictures to be taken
    void set_amount(int& am)
    { m_picAmount = am; }

    // Returns the amount of ephemerides
    const std::vector<Ephemeris>& getEphs() const
    { return m_ephemerides; }

    // Returns the amount of pictures
    const std::vector<Picture>& getPics() const
    { return m_pictures; }

    

    // Calculates the number of ephemerides covered by a single picture
    const int ephemeris_in_picture(float& ra, float& dec);

    // Prints observation targets to the console
    void export_observation_targets(bool copy_cpb);

    // Inserts data for a single ephemeris
    // \param str the single ephemeris string from the offsets link
    void insert_data(std::string& str);

    // Inserts a picture in the dataset
    // \param ra right ascencion
    // \param dec declination
    void insert_picture(float& ra, float& dec);

    // Removes a single picture from the dataset
    // \param index the index of the picture to be removed
    void remove_picture(int index);

    // Removes the last inserted picture
    void undo_picture();

    // Clears all pictures
    void clear_pictures();

    // Determines which ephemeris datapoint is closest to given location
    // \param ra right ascension of the location
    // \param dec declination of the location
    const int closest_ephemeris_index(float& ra, float& dec);

    // Determines which picture is closest to given location
    // \param ra right ascension of the location
    // \param dec declination of the location
    const int closest_picture_index(float& ra, float& dec);



    // resets the database
    void reset();

    // how many ephemerides are covered by all picture areas
    // \returns coverage percentage
    const float calculateSelected() const;

    // Figures where is the center of the ephemeris dataset
    // \param[out] minRa_ptr pointer where smallest right ascension OFFSET value will be stored
    // \param[out] maxRa_ptr pointer where largest right ascension OFFSET value will be stored
    // \param[out] minDec_ptr pointer where smallest declination OFFSET value will be stored
    // \param[out] maxDec_ptr pointer where largest declination OFFSET value will be stored
    // \return center right ascension / declination
    const std::tuple<float, float> getCenter(
        int* minRa_ptr = nullptr, int* maxRa_ptr = nullptr,
        int* minDec_ptr = nullptr, int* maxDec_ptr = nullptr
    ) const;
    
    // Figures where the farthest points of the ephemeris data are
    // \return edge right ascension / declination
    const std::tuple<float, float> getEdges() const;

    // the command that fills the database from the object offsets link
    // \param link object offsets link
    int fill_database(std::string& link);

};