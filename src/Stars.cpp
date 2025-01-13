//----------------------------------------------------------

#include "Stars.hpp"

//----------------------------------------------------------

static int callback(void* data, int argc, char** argv, char** azColName) 
{ 
    std::vector<Star>* sp = (std::vector<Star>*)data;
    try{
        Star s{std::stoi(argv[0]), std::stof(argv[1]), std::stof(argv[2]), std::stof(argv[3]), argv[4]};
        sp->emplace_back(s);
    }
    catch (error_t e){
        std::cout << "Error: invalid data in database!" << std::endl;
    }
    return 0;
} 

Stars::Stars(const std::string& path)
{
    int code = sqlite3_open(path.c_str(), &db);
    if (code) throw mpcsError::DbError(fmt::format("Cant open database at {}: \n{}", path, sqlite3_errmsg(db)));
    else std::cout << "Info: Opened Database Successfully!" << std::endl; 
}

Stars::~Stars()
{
    sqlite3_close(db);
}

void Stars::fetchStarsbyMag(float maxmag, float minmag /*= -100*/)
{
    stars.clear(); // this isnt the way to do this later on, its wasteful, there should be some "cache" 
    std::string sql = "SELECT * FROM STAR WHERE MAG BETWEEN " +std::to_string(minmag)+ " AND " +std::to_string(maxmag);
    char* message;
    int code = sqlite3_exec(db, sql.c_str(), callback, (void*)&stars, &message);
    if (code) throw mpcsError::DbError(fmt::format("Cant do sql operation:\nOPERATION:\t{}\nERROR:\t{}", sql, message));
    else std::cout << "Info: Opened Database Successfully!" << std::endl; 
}