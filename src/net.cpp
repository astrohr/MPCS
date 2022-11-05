// net.cpp -------------------------------------------------
// this file defines all things declared in net.hpp

#include "net.hpp"

//----------------------------------------------------------



std::vector<std::string> g_allowed_links; //this gets populated in default_variables()

static size_t progress_callback(void* approx_size, double dltotal, double dlnow, double ultotal, double ulnow){
    if (dltotal <= 0.0) dltotal = *((double*)approx_size);
    if (dltotal > 0.0){
        std::cout << "\r[";
        int bar_size = 45;
        int fragments = round(dlnow/dltotal*bar_size);
        for(int i = 0; i < bar_size; i++){
            if (i<fragments) std::cout << '#';
            else std::cout << '-';
        }
        int perc = dlnow/dltotal*100;
        std::cout << "] " << ((perc > 100) ? 100 : perc) << "%  ";
    }
    std::cout << "(" << dlnow << " B)" << std::flush;
    return 0;
}

size_t read_curl_data(char* ptr, size_t size, size_t nmemb, std::vector<std::string>* userdata){
    size_t bytes = size*nmemb;

    for(char *i = ptr; i-ptr < bytes; i++){
        (*userdata)[0] += *i;
        if(*i == '\n'){
            (*userdata).emplace_back((*userdata)[0]);
            (*userdata)[0] = "";
        }
    }
    return bytes; // returns the number of bytes passed to the function
}

// defaults from header: size = 0.0
int get_html(std::string link, std::vector<std::string>* userdata, double size, int milis){
    std::cout << "Downloading data..." << std::endl;
    // Check if the link type is allowed
    bool match = false;
    for(int i = 0; i < g_allowed_links.size() && !match; i++){
        bool ac = true;
        for(int j = 0; j < g_allowed_links[i].size(); j++){
            if (j >= link.size() || link[j] != g_allowed_links[i][j]){
                ac = false;
                break;
            }
        }
        if(ac) match = true;
    }
    if (!match){
        std::cout << "Error: link not allowed" << std::endl;
        return 2;
    }


    std::map<CURLcode, int> responsovi;
    sf::Clock clock;
    while(clock.getElapsedTime().asMilliseconds() < milis){
        CURL *curl = curl_easy_init();
        (*userdata).emplace_back("");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_curl_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, userdata);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void*)&size);
        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());

        CURLcode res = curl_easy_perform(curl);
        
        curl_easy_cleanup(curl);
        
        if (responsovi.find(res) != responsovi.end()) responsovi[res]++;
        else responsovi[res] = 1;

        if (res == CURLE_OK) break;
        else if (res == CURLE_SSL_CONNECT_ERROR) milis = 100000;
        std::cout << " " << (milis/1000)-(int)clock.getElapsedTime().asSeconds() << " s   " << std::flush;
    }
    
    int returnvalue;
    if (responsovi.find(CURLE_OK) != responsovi.end()){
        std::cout << "\nDownload success\n" << std::endl;
        returnvalue = 0;
    }
    else{
        CURLcode res; int mx = 0;
        for (auto x : responsovi){
            if (x.second > mx){
                res = x.first;
                mx = x.second;
            }
        }
        std::cout << "\nDownload failed: " << curl_easy_strerror(res) << std::endl;
        returnvalue = 1;
    }
    return returnvalue;
}

