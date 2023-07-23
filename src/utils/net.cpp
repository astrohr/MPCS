//----------------------------------------------------------

#include "utils.hpp"

//----------------------------------------------------------


std::vector<std::string> g_allowed_links = {
    "https://cgi.minorplanetcenter.net/",
    "https://www.minorplanetcenter.net/",
    "http://cgi.minorplanetcenter.net/",
};

void get_html(std::string& link, std::vector<std::string>& raw, int milis){
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
        throw mpcsError::DownloadFail(r.error.message);
    else if (r.status_code >= 400) {
        throw mpcsError::DownloadFail(fmt::format("Error {} making request", r.status_code));
    }
    
    // separate the response at every newline
    raw.clear();
    std::istringstream iss(r.text);
    std::string line;
    while (std::getline(iss, line)) raw.push_back(line);

}