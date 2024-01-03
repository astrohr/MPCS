//----------------------------------------------------------

#include "utils.hpp"

//----------------------------------------------------------


std::vector<std::string> g_allowed_links = {
    "https://cgi.minorplanetcenter.net/",
    "https://www.minorplanetcenter.net/",
    "http://cgi.minorplanetcenter.net/",
    "https://minorplanetcenter.net/",
    "https://aa.usno.navy.mil/api/",
};

std::string get_html(std::string& link, int milis){
    // Check if the link type is allowed by comparing to the strings in g_allowed_links
    bool match = false;
    for(int i = 0; i < g_allowed_links.size() && !match; i++)
        if (link.substr(0, g_allowed_links[i].size()) == g_allowed_links[i]) match = true;
    
    if (!match)
        throw mpcsError::ForbiddenLink(fmt::format("Link \"{}\" is not allowed", link));

    using namespace cpr;
    // there is a lot of namespace cpr calls here so yeah

    int x = 0;
    // get the url data
    Response r = Get(
        Url(link),
        Timeout(milis),
        ProgressCallback( // callback function
            [&](cpr_off_t downloadTotal, cpr_off_t downloadNow, cpr_off_t uploadTotal, cpr_off_t uploadNow, intptr_t userdata) -> bool {
                char prog;
                if (x==0) prog = '|';
                else if (x==1) prog = '/';
                else if (x==2) prog = '-';
                else prog = '\\';
                fmt::print("\rDownloading data... {}       ", prog);
                x = (x+1)%4;
                return true;
            }
        ) 
    );

    // check for errors
    if(r.status_code == 0)
        throw mpcsError::DownloadFail(fmt::format("Error: {}\nLink which returned error: {} ", r.error.message, link));
    else if (r.status_code >= 400) {
        throw mpcsError::DownloadFail(fmt::format("Error {} making request\nLink which returned error: {}", r.status_code, link));
    }

    fmt::println("\rDownload success :D               ");

    return r.text;
}

void get_html(std::string& link, std::vector<std::string>& raw, int milis){
    std::string rawstr = get_html(link, milis);
    
    // separate the response at every newline
    raw.clear();
    std::istringstream iss(rawstr);
    std::string line;
    while (std::getline(iss, line)) raw.push_back(line);   
}