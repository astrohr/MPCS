//----------------------------------------------------------

#include "utils.hpp"

//----------------------------------------------------------


std::vector<std::string> g_allowed_links = {
    "https://cgi.minorplanetcenter.net/",
    "https://www.minorplanetcenter.net/",
    "http://cgi.minorplanetcenter.net/",
};


// https://curl.se/libcurl/c/CURLOPT_PROGRESSFUNCTION.html
static size_t progress_callback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow){
    char prog; int progamm = *((int*)clientp);
    if (progamm==0) prog = '|';
    else if (progamm==1) prog = '/';
    else if (progamm==2) prog = '-';
    else prog = '\\';
    fmt::print("\rDownloading data... {}       ", prog);
    //ammount ++, mod 4
    (*((int*)clientp))=(progamm+1)%4;
    return 0;
}

// functions that reads data from a url location
// https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
size_t read_curl_data(char* ptr, size_t size, size_t nmemb, std::vector<std::string>* userdata){
    size_t bytes = size*nmemb;

    // saves the data in the user data variable in a way that every line of html gets its own string
    for(char *i = ptr; i-ptr < bytes; i++){
        //saves all data on the n-th line in the first element
        (*userdata)[0] += *i;
        if(*i == '\n'){
            //places the first element on the end of the array and clears it
            userdata->emplace_back((*userdata)[0]);
            (*userdata)[0] = "";
        }
    }
    return bytes; // returns the number of bytes passed to the function
}

void utils::get_html(std::string& link, std::vector<std::string>& userdata, int milis){
    // Check if the link type is allowed by comparing to the strings in g_allowed_links
    bool match = false;
    for(int i = 0; i < g_allowed_links.size() && !match; i++)
        if (link.substr(0, g_allowed_links[i].size()) == g_allowed_links[i]) match = true;
    
    if (!match)
        throw utils::ForbiddenLink(fmt::format("Link \"{}\" is not allowed", link));


    std::map<CURLcode, int> responsovi;
    sf::Clock clock;
    //the loop that keeeps trying to connect to a site for some ammount of time
    while(clock.getElapsedTime().asMilliseconds() < milis){
        //prepare userdata fo writing in it
        userdata.emplace_back("");
        //setting up the curl request
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_curl_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &userdata);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, (long)1);
        int progcall = 0; // measures how many times progressdata function is called, mod 4
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void*)&progcall);
        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());

        CURLcode res = curl_easy_perform(curl); //execute request
        curl_easy_cleanup(curl);
        
        //save the response in a map that measures the ammounts of specific responses
        if (responsovi.find(res) != responsovi.end()) responsovi[res]++;
        else responsovi[res] = 1;

        //handle the response
        if (res == CURLE_OK) break;
        else if (res == CURLE_SSL_CONNECT_ERROR) milis = 60000;

        //if response wasnt CURLE_OK, then display for how much time will the program keep trying
        fmt::print(" {:.1f} s   ", (milis/1000)-clock.getElapsedTime().asSeconds());

        //clear userdata since the request failed and we have to retry
        userdata.clear();
    }
    
    //if there is CURLE_OK, all is fine
    if (responsovi.find(CURLE_OK) != responsovi.end())
        fmt::print("\rDownload success :D          \n");
    else{
        std::string error = "\rDownload fail:\n";
        for (auto x : responsovi)
            error += fmt::format("{} times: {}\n", x.second, curl_easy_strerror(x.first));

        throw utils::DownloadFail(error);
    }
}

